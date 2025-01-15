#include "Shape.h"
#include "Device.h"

namespace candlewick {

Shape::Shape(Shape &&other)
    : device(other.device), masterVertexBuffer(other.masterVertexBuffer),
      masterIndexBuffer(other.masterIndexBuffer),
      meshes_(std::move(other.meshes_)),
      materials_(std::move(other.materials_)) {
  layout_ = std::move(other.layout_);
  device = other.device;
  other.masterVertexBuffer = NULL;
  other.masterIndexBuffer = NULL;
  other.device = NULL;
}

Shape Shape::createShapeFromDatas(const Device &device,
                                  std::span<const MeshData> meshDatas,
                                  bool upload) {
  using IndexType = MeshData::IndexType;
  std::vector<Mesh> meshes;
  std::vector<PbrMaterialData> materials;
  const auto &layout = meshDatas[0].layout();
  Uint32 numVertices = 0, numIndices = 0;
  for (size_t i = 0; i < meshDatas.size(); i++) {
    numVertices += meshDatas[i].numVertices();
    numIndices += meshDatas[i].numIndices();
  }

  SDL_GPUBufferCreateInfo vtxInfo{.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
                                  .size = numVertices * layout.vertexSize(),
                                  .props = 0};
  SDL_GPUBufferCreateInfo idxInfo;

  if (numIndices > 0) {
    idxInfo = {.usage = SDL_GPU_BUFFERUSAGE_INDEX,
               .size = Uint32(numIndices * sizeof(IndexType)),
               .props = 0};
  }

  SDL_GPUBuffer *masterVertexBuffer = SDL_CreateGPUBuffer(device, &vtxInfo);
  SDL_GPUBuffer *masterIndexBuffer =
      (numIndices > 0) ? SDL_CreateGPUBuffer(device, &idxInfo) : NULL;

  Uint32 vertexOffset = 0, indexOffset = 0;
  for (size_t i = 0; i < meshDatas.size(); i++) {
    meshes.emplace_back(createMesh(meshDatas[i], masterVertexBuffer,
                                   vertexOffset, masterIndexBuffer, indexOffset,
                                   false));
    materials.push_back(meshDatas[i].material);
    vertexOffset += meshDatas[i].numVertices() * layout.vertexSize();
    indexOffset += meshDatas[i].numIndices() * sizeof(IndexType);
  }

  if (upload) {
    for (size_t i = 0; i < meshes.size(); i++) {
      uploadMeshToDevice(device, meshes[i], meshDatas[i]);
    }
  }
  return Shape(device, masterVertexBuffer, masterIndexBuffer, layout,
               std::move(meshes), std::move(materials));
}

Shape Shape::createShapeFromDatas(const Device &device,
                                  std::vector<MeshData> &&meshDatas) {
  std::span<const MeshData> view{meshDatas};
  return createShapeFromDatas(device, view, true);
}

void Shape::release() {
  if (device && *device) {
    SDL_ReleaseGPUBuffer(*device, masterVertexBuffer);
    SDL_ReleaseGPUBuffer(*device, masterIndexBuffer);
    masterVertexBuffer = NULL;
    masterIndexBuffer = NULL;
    device = NULL;
  }
}

} // namespace candlewick
