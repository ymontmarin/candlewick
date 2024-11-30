#include "Device.h"
#include "MeshGroup.h"
#include "../utils/MeshData.h"

#include <SDL3/SDL_assert.h>

namespace candlewick {

void MeshGroup::releaseBuffers(const Device &device) {
  SDL_ReleaseGPUBuffer(device, masterVertexBuffer);
  if (masterIndexBuffer) {
    SDL_ReleaseGPUBuffer(device, masterIndexBuffer);
  }
}

MeshGroup::MeshGroup(const Device &device, std::span<MeshData> meshDatas)
    : meshes(), meshDatas(meshDatas) {
  using Vertex = MeshData::Vertex;
  using IndexType = MeshData::IndexType;
  Uint32 numVertices = 0, numIndices = 0;
  for (size_t i = 0; i < meshDatas.size(); i++) {
    numVertices += meshDatas[i].numVertices();
    numIndices += meshDatas[i].numIndices();
  }

  SDL_GPUBufferCreateInfo vtxInfo{.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
                                  .size = Uint32(numVertices * sizeof(Vertex)),
                                  .props = 0};
  SDL_GPUBufferCreateInfo idxInfo;

  if (numIndices > 0) {
    idxInfo = {.usage = SDL_GPU_BUFFERUSAGE_INDEX,
               .size = Uint32(numIndices * sizeof(IndexType)),
               .props = 0};
  }

  masterVertexBuffer = SDL_CreateGPUBuffer(device, &vtxInfo);
  masterIndexBuffer =
      (numIndices > 0) ? SDL_CreateGPUBuffer(device, &idxInfo) : NULL;

  Uint32 vertexOffset = 0, indexOffset = 0;
  for (size_t i = 0; i < meshDatas.size(); i++) {
    meshes.emplace_back(convertToMesh(meshDatas[i], masterVertexBuffer,
                                      vertexOffset, masterIndexBuffer,
                                      indexOffset, false));
    vertexOffset += sizeof(Vertex) * meshDatas[i].numVertices();
    indexOffset += sizeof(IndexType) * meshDatas[i].numIndices();
  }
}

void uploadMeshGroupToDevice(const Device &device, const MeshGroup &group) {
  for (size_t i = 0; i < group.meshes.size(); i++) {
    uploadMeshToDevice(device, group.meshes[i], group.meshDatas[i]);
  }
}
} // namespace candlewick
