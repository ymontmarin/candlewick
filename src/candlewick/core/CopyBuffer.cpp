#include "CopyBuffer.h"

#include "Mesh.h"
#include "Device.h"
#include "../utils/MeshData.h"

namespace candlewick {

void uploadMesh(const Device &device, const Mesh &mesh,
                const MeshData &meshData) {
  SDL_GPUCommandBuffer *command_buffer = SDL_AcquireGPUCommandBuffer(device);
  SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(command_buffer);

  SDL_GPUTransferBuffer *transfer_buffer;
  {
    // vertex data
    Uint32 vertex_payload_size =
        meshData.numVertices() * sizeof(MeshData::Vertex);

    SDL_GPUTransferBufferCreateInfo transfer_buffer_desc{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = vertex_payload_size,
        .props = 0,
    };
    transfer_buffer =
        SDL_CreateGPUTransferBuffer(device, &transfer_buffer_desc);
    void *map = SDL_MapGPUTransferBuffer(device, transfer_buffer, false);
    SDL_memcpy(map, meshData.vertexData.data(), vertex_payload_size);
    SDL_UnmapGPUTransferBuffer(device, transfer_buffer);

    SDL_GPUTransferBufferLocation src_location{
        .transfer_buffer = transfer_buffer, .offset = 0};
    SDL_GPUBufferRegion dst_region{.buffer = mesh.vertexBuffers[0],
                                   .offset = mesh.vertexBufferOffsets[0],
                                   .size = vertex_payload_size};
    SDL_UploadToGPUBuffer(copy_pass, &src_location, &dst_region, false);
    SDL_ReleaseGPUTransferBuffer(device, transfer_buffer);
  }

  if (mesh.isIndexed()) {
    // now upload the index data
    Uint32 index_payload_size =
        meshData.numIndices() * sizeof(MeshData::IndexType);

    SDL_GPUTransferBufferCreateInfo transfer_buffer_desc{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = index_payload_size,
        .props = 0};
    transfer_buffer =
        SDL_CreateGPUTransferBuffer(device, &transfer_buffer_desc);
    void *map = SDL_MapGPUTransferBuffer(device, transfer_buffer, false);
    SDL_memcpy(map, meshData.indexData.data(), index_payload_size);
    SDL_UnmapGPUTransferBuffer(device, transfer_buffer);
    SDL_GPUTransferBufferLocation src_location{
        .transfer_buffer = transfer_buffer, .offset = mesh.indexOffset};
    SDL_GPUBufferRegion dst_region{.buffer = mesh.indexBuffer,
                                   .offset = mesh.indexBufferOffset,
                                   .size = index_payload_size};
    SDL_UploadToGPUBuffer(copy_pass, &src_location, &dst_region, false);
    SDL_ReleaseGPUTransferBuffer(device, transfer_buffer);
  }

  SDL_EndGPUCopyPass(copy_pass);
  SDL_SubmitGPUCommandBuffer(command_buffer);
}

} // namespace candlewick
