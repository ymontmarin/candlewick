#include "Mesh.h"

#include "MeshLayout.h"
#include "./errors.h"

namespace candlewick {

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

} // namespace candlewick
