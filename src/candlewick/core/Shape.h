#pragma once

#include "Device.h"
#include "Mesh.h"
#include "../utils/MeshData.h"
#include "../utils/MaterialData.h"

namespace candlewick {

/// \brief Class containing the mesh \c Mesh handles for a set of meshes with
/// materials.
/// \invariant All the meshes share a given master vertex buffer and master
/// index buffer, to allow batching of draw calls.
class Shape {
private:
  SDL_GPUDevice *device;
  SDL_GPUBuffer *masterVertexBuffer;
  SDL_GPUBuffer *masterIndexBuffer;

  std::vector<Mesh> meshes_;
  std::vector<PbrMaterialData> materials_;
  MeshLayout layout_;

  Shape(SDL_GPUDevice *dev, SDL_GPUBuffer *vb, SDL_GPUBuffer *ib, MeshLayout l,
        std::vector<Mesh> &&m, std::vector<PbrMaterialData> &&mat)
      : device(dev), masterVertexBuffer(vb), masterIndexBuffer(ib),
        meshes_(std::move(m)), materials_(std::move(mat)), layout_(l) {}

public:
  enum { MATERIAL_SLOT = 0 };

  Shape(const Shape &) = delete;
  Shape(Shape &&) = default;

  SDL_GPUBufferBinding getVertexBinding() const {
    return {.buffer = masterVertexBuffer, .offset = 0};
  }
  SDL_GPUBufferBinding getIndexBinding() const {
    return {.buffer = masterIndexBuffer, .offset = 0};
  }
  const std::vector<Mesh> &meshes() const { return meshes_; }
  const std::vector<PbrMaterialData> &materials() const { return materials_; }
  const MeshLayout &layout() const { return layout_; }
  bool hasMaterials() const { return !materials_.empty(); }

  friend class ShapeBuilder;

  void release() {
    SDL_ReleaseGPUBuffer(device, masterVertexBuffer);
    SDL_ReleaseGPUBuffer(device, masterIndexBuffer);
  }

  static Shape createShapeFromDatas(const Device &device,
                                    std::span<MeshData> meshDatas,
                                    bool upload = false);
};

inline Shape Shape::createShapeFromDatas(const Device &device,
                                         std::span<MeshData> meshDatas,
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
    meshes.emplace_back(convertToMesh(meshDatas[i], masterVertexBuffer,
                                      vertexOffset, masterIndexBuffer,
                                      indexOffset, false));
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
} // namespace candlewick
