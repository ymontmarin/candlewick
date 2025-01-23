#include "Mesh.h"

#include "MeshLayout.h"
#include "./errors.h"

#include <cassert>

namespace candlewick {

MeshView::MeshView(const MeshView &parent, Uint32 subVertexOffset,
                   Uint32 subVertexCount, Uint32 subIndexOffset,
                   Uint32 subIndexCount)
    : vertexBuffers(parent.vertexBuffers), indexBuffer(parent.indexBuffer),
      vertexOffset(parent.vertexOffset + subVertexOffset),
      vertexCount(subVertexCount),
      indexOffset(parent.indexOffset + subIndexOffset),
      indexCount(subIndexCount) {
  // assumption: parent MeshView is validated
  assert(validateMeshView(*this));
  assert(subVertexOffset + subVertexCount <= parent.vertexCount);
  assert(subIndexOffset + subIndexCount <= parent.indexCount);
}

Mesh::Mesh(NoInitT) : layout() {}

Mesh::Mesh(const MeshLayout &layout) : layout(layout) {
  const Uint32 count = layout.numBuffers();
  vertexBuffers.resize(count, nullptr);
}

Mesh &Mesh::bindVertexBuffer(Uint32 slot, SDL_GPUBuffer *buffer) {
  for (std::size_t i = 0; i < layout.numBuffers(); i++) {
    if (layout.toVertexInputState().vertex_buffer_descriptions[i].slot ==
        slot) {
      if (vertexBuffers[i]) {
        throw InvalidArgument(
            "Rebinding vertex buffer to already occupied slot.");
      }
      vertexBuffers[i] = buffer;
      return *this;
    }
  }
  CDW_UNREACHABLE_ASSERT("Binding slot not found!");
  std::terminate();
}

Mesh &Mesh::setIndexBuffer(SDL_GPUBuffer *buffer) {
  indexBuffer = buffer;
  return *this;
}

void Mesh::release(SDL_GPUDevice *device) {
  for (std::size_t i = 0; i < vertexBuffers.size(); i++) {
    if (vertexBuffers[i])
      SDL_ReleaseGPUBuffer(device, vertexBuffers[i]);
    vertexBuffers[i] = nullptr;
  }

  if (isIndexed()) {
    SDL_ReleaseGPUBuffer(device, indexBuffer);
    indexBuffer = nullptr;
  }
}

MeshView &Mesh::addView(Uint32 vertexOffset, Uint32 vertexSubCount,
                        Uint32 indexOffset, Uint32 indexSubCount) {
  MeshView v;
  v.vertexBuffers = vertexBuffers;
  v.indexBuffer = indexBuffer;
  v.vertexOffset = vertexOffset;
  v.vertexCount = vertexSubCount;
  v.indexOffset = indexOffset;
  v.indexCount = indexSubCount;

  return views_.emplace_back(v);
}

} // namespace candlewick
