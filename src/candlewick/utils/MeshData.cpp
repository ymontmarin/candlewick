#include "MeshData.h"
#include "../core/Device.h"
#include "../core/Mesh.h"

#include <SDL3/SDL_assert.h>

namespace candlewick {

Mesh convertToMesh(const MeshData &meshData, SDL_GPUBuffer *vertexBuffer,
                   Uint64 vertexOffset, SDL_GPUBuffer *indexBuffer,
                   Uint64 indexOffset, bool takeOwnership) {
  using Vertex = MeshData::Vertex;
  Mesh mesh{MeshLayout{SDL_GPU_PRIMITIVETYPE_TRIANGLELIST}
                .addBinding(0, sizeof(Vertex))
                .addAttribute(0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                              offsetof(Vertex, pos))
                .addAttribute(1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                              offsetof(Vertex, normal))};

  mesh.addVertexBuffer(0, vertexBuffer, vertexOffset, takeOwnership)
      .setIndexBuffer(indexBuffer, indexOffset, takeOwnership);
  SDL_assert(mesh.isIndexed());
  mesh.count = meshData.numIndices();
  return mesh;
}

Mesh convertToMesh(const Device &device, const MeshData &meshData) {
  using Vertex = MeshData::Vertex;
  using IndexType = MeshData::IndexType;

  SDL_GPUBufferCreateInfo vtxInfo{
      .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
      .size = static_cast<Uint32>(sizeof(Vertex) * meshData.numVertices()),
      .props = 0};

  SDL_GPUBufferCreateInfo indexInfo{
      .usage = SDL_GPU_BUFFERUSAGE_INDEX,
      .size = static_cast<Uint32>(sizeof(IndexType) * meshData.numIndices()),
      .props = 0};

  SDL_GPUBuffer *vertexBuffer = SDL_CreateGPUBuffer(device, &vtxInfo);
  SDL_GPUBuffer *indexBuffer = SDL_CreateGPUBuffer(device, &indexInfo);
  return convertToMesh(meshData, vertexBuffer, 0, indexBuffer, 0, true);
}

MeshData::MeshData(std::vector<Vertex> vertexData,
                   std::vector<IndexType> indexData)
    : vertexData(std::move(vertexData)), indexData(std::move(indexData)) {}

} // namespace candlewick
