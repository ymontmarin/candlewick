#include "Mesh.h"

#include "Device.h"
#include "MeshLayout.h"
#include "./errors.h"

namespace candlewick {

Mesh::Mesh(MeshLayout layout) : _layout(std::move(layout)) {
  const Uint32 count = _layout.toVertexInputState().num_vertex_buffers;
  vertexBuffers.resize(count);
  vertexBufferOffsets.resize(count);
}

std::size_t Mesh::addVertexBufferImpl(Uint32 slot, SDL_GPUBuffer *buffer,
                                      Uint64 offset) {
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

Mesh &Mesh::addVertexBuffer(Uint32 slot, SDL_GPUBuffer *buffer, Uint64 offset) {
  addVertexBufferImpl(slot, buffer, offset);
  return *this;
}

Mesh &Mesh::setIndexBuffer(SDL_GPUBuffer *buffer, Uint64 offset) {
  indexBuffer = buffer;
  indexBufferOffset = offset;
  return *this;
}

void Mesh::releaseBuffers(const Device &device) {
  for (auto &buf : vertexBuffers) {
    SDL_ReleaseGPUBuffer(device, buf);
    buf = nullptr;
  }

  if (isIndexed()) {
    SDL_ReleaseGPUBuffer(device, indexBuffer);
    indexBuffer = nullptr;
  }
}

} // namespace candlewick
