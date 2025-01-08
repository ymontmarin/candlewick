#include "../core/Device.h"
#include "MeshGroup.h"
#include "MeshData.h"

#include <SDL3/SDL_assert.h>

namespace candlewick {

MeshGroup createMeshGroup(const Device &device, std::span<MeshData> meshDatas,
                          bool uploadToDevice) {
  using IndexType = MeshData::IndexType;
  MeshGroup meshes;
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
    vertexOffset += meshDatas[i].numVertices() * layout.vertexSize();
    indexOffset += meshDatas[i].numIndices() * sizeof(IndexType);
  }

  if (uploadToDevice) {
    for (size_t i = 0; i < meshes.size(); i++) {
      uploadMeshToDevice(device, meshes[i], meshDatas[i]);
    }
  }

  return meshes;
}

void releaseMeshGroup(const Device &device, MeshGroup &group) {
  for (auto &mesh : group) {
    mesh.releaseOwnedBuffers(device);
  }
}

} // namespace candlewick
