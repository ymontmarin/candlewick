#include "MeshData.h"
#include "../core/Device.h"
#include "../core/Mesh.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_log.h>

namespace candlewick {

MeshData::MeshData(NoInitT) {}

MeshData::MeshData(SDL_GPUPrimitiveType primitiveType, MeshLayout layout,
                   std::vector<char> vertexData,
                   std::vector<IndexType> indexData)
    : primitiveType(primitiveType), vertexData(std::move(vertexData), layout),
      indexData(std::move(indexData)) {}

Mesh createMesh(const Device &device, const MeshData &meshData) {
  using IndexType = MeshData::IndexType;
  auto &layout = meshData.layout();
  SDL_GPUBufferCreateInfo vtxInfo{
      .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
      .size = Uint32(layout.vertexSize() * meshData.numVertices()),
      .props = 0};

  SDL_GPUBuffer *vertexBuffer = SDL_CreateGPUBuffer(device, &vtxInfo);
  SDL_GPUBuffer *indexBuffer = NULL;
  if (meshData.isIndexed()) {
    SDL_GPUBufferCreateInfo indexInfo{
        .usage = SDL_GPU_BUFFERUSAGE_INDEX,
        .size = Uint32(sizeof(IndexType) * meshData.numIndices()),
        .props = 0};
    indexBuffer = SDL_CreateGPUBuffer(device, &indexInfo);
  }
  return createMesh(meshData, vertexBuffer, indexBuffer);
}

Mesh createMesh(const MeshData &meshData, SDL_GPUBuffer *vertexBuffer,
                SDL_GPUBuffer *indexBuffer) {
  Mesh mesh{meshData.layout()};

  mesh.bindVertexBuffer(0, vertexBuffer);
  mesh.vertexCount = meshData.numVertices();
  mesh.indexCount = meshData.numIndices();
  if (meshData.isIndexed()) {
    mesh.setIndexBuffer(indexBuffer);
  }
  return mesh;
}

std::pair<Mesh, std::vector<MeshView>>
createMeshFromBatch(const Device &device, std::span<const MeshData> meshDatas,
                    bool upload) {
  using IndexType = MeshData::IndexType;
  // index type size, in bytes
  constexpr Uint32 indexSize = sizeof(IndexType);
  const size_t N = meshDatas.size();
  SDL_assert(N > 0);
  std::vector<MeshView> views;
  views.resize(N);
  auto layout = meshDatas[0].layout();

  Uint32 numVertices = 0, numIndices = 0;
  for (auto &data : meshDatas) {
    numVertices += data.numVertices();
    numIndices += data.numIndices();
  }
  SDL_assert(layout.numBuffers() == 1);

  SDL_GPUBufferCreateInfo vtxInfo{.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
                                  .size =
                                      Uint32(layout.vertexSize() * numVertices),
                                  .props = 0};

  SDL_GPUBufferCreateInfo idxInfo;

  if (numIndices > 0) {
    idxInfo = {.usage = SDL_GPU_BUFFERUSAGE_INDEX,
               .size = numIndices * indexSize,
               .props = 0};
  }

  auto masterVertexBuffer = SDL_CreateGPUBuffer(device, &vtxInfo);
  auto masterIndexBuffer =
      (numIndices > 0) ? SDL_CreateGPUBuffer(device, &idxInfo) : NULL;
  Mesh mesh{layout};
  mesh.vertexCount = numVertices;
  mesh.indexCount = numIndices;
  mesh.bindVertexBuffer(0, masterVertexBuffer)
      .setIndexBuffer(masterIndexBuffer);

  Uint32 vertexOffset = 0, indexOffset = 0;
  for (size_t i = 0; i < N; i++) {
    views[i] = {.vertexBuffers = mesh.vertexBuffers,
                .vertexOffsets = {vertexOffset},
                .vertexCount = meshDatas[i].numVertices(),
                .indexBuffer = mesh.indexBuffer,
                .indexOffset = indexOffset,
                .indexCount = meshDatas[i].numIndices()};
    vertexOffset += meshDatas[i].numVertices();
    indexOffset += meshDatas[i].numIndices();
    if (upload)
      uploadMeshToDevice(device, views[i], meshDatas[i]);
  }
  return {std::move(mesh), std::move(views)};
}

void uploadMeshToDevice(const Device &device, const MeshView &meshView,
                        const MeshData &meshData) {

  SDL_GPUCommandBuffer *upload_command_buffer;
  SDL_GPUTransferBuffer *transfer_buffer;
  SDL_GPUCopyPass *copy_pass;

  upload_command_buffer = SDL_AcquireGPUCommandBuffer(device);
  copy_pass = SDL_BeginGPUCopyPass(upload_command_buffer);

  constexpr Uint32 indexSize = sizeof(MeshData::IndexType);
  auto &layout = meshData.layout();
  const Uint32 vertex_payload_size =
      meshData.numVertices() * layout.vertexSize();
  const Uint32 index_payload_size = meshData.numIndices() * indexSize;
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
          .buffer = meshView.vertexBuffers[0],
          .offset = meshView.vertexOffsets[0] * layout.vertexSize(),
          .size = vertex_payload_size,
      };
      SDL_UploadToGPUBuffer(copy_pass, &src_location, &dst_region, false);
    }
    // copy indices
    if (meshView.isIndexed()) {
      SDL_memcpy(map + vertex_payload_size, meshData.indexData.data(),
                 index_payload_size);
      SDL_GPUTransferBufferLocation src_location{
          .transfer_buffer = transfer_buffer,
          .offset = vertex_payload_size,
      };
      SDL_GPUBufferRegion dst_region{
          .buffer = meshView.indexBuffer,
          .offset = meshView.indexOffset * indexSize,
          .size = index_payload_size,
      };
      SDL_UploadToGPUBuffer(copy_pass, &src_location, &dst_region, false);
    }
    SDL_UnmapGPUTransferBuffer(device, transfer_buffer);
  }
  SDL_ReleaseGPUTransferBuffer(device, transfer_buffer);
  SDL_EndGPUCopyPass(copy_pass);
  if (!SDL_SubmitGPUCommandBuffer(upload_command_buffer)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "%s: Failed to submit command buffer: %s", __FILE__,
                 SDL_GetError());
    SDL_assert(false);
  }
}

std::vector<PbrMaterialData>
extractMaterials(std::span<const MeshData> meshDatas) {
  std::vector<PbrMaterialData> out;
  out.reserve(meshDatas.size());
  for (size_t i = 0; i < meshDatas.size(); i++) {
    out.push_back(meshDatas[i].material);
  }
  return out;
}

} // namespace candlewick
