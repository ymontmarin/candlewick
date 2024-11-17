#include "Mesh.h"

#include "Device.h"
#include "MeshLayout.h"
#include "./errors.h"

namespace candlewick {

Mesh::Mesh(NoInitT) : _layout(SDL_GPUPrimitiveType(0)) {}

Mesh::Mesh(MeshLayout layout) : _layout(std::move(layout)) {
  const Uint32 count = _layout.toVertexInputState().num_vertex_buffers;
  vertexBuffers.resize(count);
  vertexBufferOffsets.resize(count);
  vertexBufferOwnerships.resize(count);
}

std::size_t Mesh::addVertexBufferImpl(Uint32 slot, SDL_GPUBuffer *buffer,
                                      Uint32 offset) {
  auto max = _layout.toVertexInputState().num_vertex_buffers;
  for (std::size_t i = 0; i < max; i++) {
    if (_layout.toVertexInputState().vertex_buffer_descriptions[i].slot ==
        slot) {
      vertexBuffers[i] = buffer;
      vertexBufferOffsets[i] = offset;
      return i;
    }
  }
  CDW_UNREACHABLE_ASSERT("Binding not found in vertex buffer.");
  return ~std::size_t{};
}

Mesh &Mesh::addVertexBuffer(Uint32 slot, SDL_GPUBuffer *buffer, Uint32 offset,
                            bool takeOwnership) {
  std::size_t idx = addVertexBufferImpl(slot, buffer, offset);
  vertexBufferOwnerships[idx] = takeOwnership ? Owned : Borrowed;
  SDL_Log("Adding vertex buffer to mesh in slot %u (offset: %u, owned: %d)",
          slot, offset, takeOwnership);
  return *this;
}

Mesh &Mesh::setIndexBuffer(SDL_GPUBuffer *buffer, Uint32 offset,
                           bool takeOwnership) {
  indexBuffer = buffer;
  indexBufferOffset = offset;
  indexBufferOwnership = takeOwnership ? Owned : Borrowed;
  return *this;
}

void Mesh::releaseOwnedBuffers(const Device &device) {
  for (std::size_t i = 0; i < vertexBuffers.size(); i++) {
    switch (vertexBufferOwnerships[i]) {
    case Owned: {
      SDL_Log("Releasing owned vertex buffer %zu", i);
      SDL_ReleaseGPUBuffer(device, vertexBuffers[i]);
      vertexBuffers[i] = nullptr;
      break;
    }
    case Borrowed:
      break;
    }
  }

  if (isIndexed() && indexBufferOwnership == Owned) {
    SDL_Log("Releasing owned index buffer");
    SDL_ReleaseGPUBuffer(device, indexBuffer);
    indexBuffer = nullptr;
  }
}

} // namespace candlewick
