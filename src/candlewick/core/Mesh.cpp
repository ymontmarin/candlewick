#include "Mesh.h"

#include "Device.h"
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

Mesh::Mesh(const Device &device, const MeshLayout &layout)
    : _device(device), layout(layout) {
  const Uint32 count = layout.numBuffers();
  vertexBuffers.resize(count, nullptr);
}

Mesh::Mesh(Mesh &&other) noexcept
    : _device(other._device), _views(std::move(other._views)),
      layout(other.layout), vertexCount(other.vertexCount),
      indexCount(other.indexCount),
      vertexBuffers(std::move(other.vertexBuffers)),
      indexBuffer(other.indexBuffer) {
  other._device = nullptr;
  other.indexBuffer = nullptr;
}

Mesh &Mesh::operator=(Mesh &&other) noexcept {
  if (this != &other) {
    if (_device) {
      release();
    }

    _device = other._device;
    _views = std::move(other._views);
    layout = std::move(other.layout);
    vertexCount = std::move(other.vertexCount);
    indexCount = std::move(other.indexCount);
    vertexBuffers = std::move(other.vertexBuffers);
    indexBuffer = std::move(other.indexBuffer);

    other._device = nullptr;
    other.vertexCount = 0u;
    other.indexCount = 0u;
    other.indexBuffer = nullptr;
  }
  return *this;
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

void Mesh::release() noexcept {
  if (!_device)
    return;

  for (std::size_t i = 0; i < vertexBuffers.size(); i++) {
    if (vertexBuffers[i])
      SDL_ReleaseGPUBuffer(_device, vertexBuffers[i]);
  }
  vertexBuffers.clear();

  if (isIndexed()) {
    SDL_ReleaseGPUBuffer(_device, indexBuffer);
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

  return _views.emplace_back(std::move(v));
}

} // namespace candlewick
