#include "MeshData.h"
#include "../core/Device.h"
#include "../core/Mesh.h"

#include <SDL3/SDL_assert.h>

namespace candlewick {

MeshData::MeshData(std::vector<Vertex> vertexData,
                   std::vector<IndexType> indexData)
    : vertexData(std::move(vertexData)), indexData(std::move(indexData)) {}

Mesh convertToMesh(const Device &device, const MeshData &meshData) {
  using Vertex = MeshData::Vertex;
  Mesh mesh{MeshLayout{SDL_GPU_PRIMITIVETYPE_TRIANGLELIST}
                .addBinding(0, sizeof(Vertex))
                .addAttribute(0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                              offsetof(Vertex, pos))
                .addAttribute(1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                              offsetof(Vertex, normal))};
  SDL_GPUBufferCreateInfo createInfo{
      .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
      .size = Uint32(sizeof(Vertex) * meshData.numVertices()),
      .props = 0};
  SDL_GPUBuffer *buf = SDL_CreateGPUBuffer(device, &createInfo);
  mesh.addVertexBuffer(0, buf, 0, true);
  mesh.count = meshData.numVertices();
  return mesh;
}

Mesh convertToMeshIndexed(const MeshData &meshData, SDL_GPUBuffer *vertexBuffer,
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

Mesh convertToMeshIndexed(const Device &device, const MeshData &meshData) {
  using Vertex = MeshData::Vertex;
  using IndexType = MeshData::IndexType;

  SDL_GPUBufferCreateInfo vtxInfo{
      .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
      .size = Uint32(sizeof(Vertex) * meshData.numVertices()),
      .props = 0};

  SDL_GPUBufferCreateInfo indexInfo{
      .usage = SDL_GPU_BUFFERUSAGE_INDEX,
      .size = Uint32(sizeof(IndexType) * meshData.numIndices()),
      .props = 0};

  SDL_GPUBuffer *vertexBuffer = SDL_CreateGPUBuffer(device, &vtxInfo);
  SDL_GPUBuffer *indexBuffer = SDL_CreateGPUBuffer(device, &indexInfo);
  return convertToMeshIndexed(meshData, vertexBuffer, 0, indexBuffer, 0, true);
}

void uploadMeshToDevice(const Device &device, const Mesh &mesh,
                        const MeshData &meshData) {

  SDL_GPUCommandBuffer *upload_command_buffer;
  SDL_GPUTransferBuffer *transfer_buffer;
  SDL_GPUCopyPass *copy_pass;

  upload_command_buffer = SDL_AcquireGPUCommandBuffer(device);
  copy_pass = SDL_BeginGPUCopyPass(upload_command_buffer);

  const Uint32 vertex_payload_size =
      meshData.numVertices() * sizeof(MeshData::Vertex);
  const Uint32 index_payload_size =
      meshData.numIndices() * sizeof(MeshData::IndexType);
  const Uint32 total_payload_size = vertex_payload_size + index_payload_size;

  SDL_GPUTransferBufferCreateInfo transfer_buffer_desc{
      .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
      .size = total_payload_size,
      .props = 0,
  };
  transfer_buffer = SDL_CreateGPUTransferBuffer(device, &transfer_buffer_desc);
  {
    std::byte *map =
        (std::byte *)SDL_MapGPUTransferBuffer(device, transfer_buffer, false);
    // copy vertices
    {
      SDL_memcpy(map, meshData.vertexData.data(), vertex_payload_size);
      SDL_GPUTransferBufferLocation src_location{
          .transfer_buffer = transfer_buffer,
          .offset = 0,
      };
      SDL_GPUBufferRegion dst_region{
          .buffer = mesh.vertexBuffers[0],
          .offset = mesh.vertexBufferOffsets[0],
          .size = vertex_payload_size,
      };
      SDL_UploadToGPUBuffer(copy_pass, &src_location, &dst_region, false);
    }
    // copy indices
    if (mesh.isIndexed()) {
      SDL_memcpy(map + vertex_payload_size, meshData.indexData.data(),
                 index_payload_size);
      SDL_GPUTransferBufferLocation src_location{
          .transfer_buffer = transfer_buffer,
          .offset = vertex_payload_size,
      };
      SDL_GPUBufferRegion dst_region{
          .buffer = mesh.indexBuffer,
          .offset = mesh.indexBufferOffset,
          .size = index_payload_size,
      };
      SDL_UploadToGPUBuffer(copy_pass, &src_location, &dst_region, false);
    }
    SDL_UnmapGPUTransferBuffer(device, transfer_buffer);
  }
  SDL_ReleaseGPUTransferBuffer(device, transfer_buffer);
  SDL_EndGPUCopyPass(copy_pass);
  SDL_SubmitGPUCommandBuffer(upload_command_buffer);
}

} // namespace candlewick
