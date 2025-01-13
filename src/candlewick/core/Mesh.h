#pragma once

#include "Core.h"
#include "Tags.h"
#include "MeshLayout.h"
#include <vector>

namespace candlewick {

/// \brief Convenience class for handling meshes: manages vertex and index
/// buffers.
struct Mesh {
  enum BufferOwnership {
    Owned,
    Borrowed,
  };

  explicit Mesh(MeshLayout layout);

  Mesh(NoInitT);
  Mesh(const Mesh &) = delete;
  Mesh(Mesh &&) noexcept = default;

  Mesh &operator=(const Mesh &) = delete;
  Mesh &operator=(Mesh &&) noexcept = default;

  const MeshLayout &layout() const { return _layout; }

  Mesh &bindVertexBuffer(Uint32 slot, SDL_GPUBuffer *buffer, Uint32 offset,
                         BufferOwnership ownership = Borrowed);
  Mesh &setIndexBuffer(SDL_GPUBuffer *buffer, Uint32 offset,
                       BufferOwnership ownership = Borrowed);

  bool isIndexed() const { return indexBuffer != NULL; }
  bool isCountSet() const { return count != ~Uint32{}; }

  void releaseOwnedBuffers(SDL_GPUDevice *device);

  SDL_GPUBufferBinding getVertexBinding(Uint32 slot) const {
    return {.buffer = vertexBuffers[slot], .offset = vertexBufferOffsets[slot]};
  }

  SDL_GPUBufferBinding getIndexBinding() const {
    return {.buffer = indexBuffer, .offset = indexBufferOffset};
  }

private:
  /// Add a vertex buffer corresponding to binding slot @p binding,
  /// using the pre-allocated @p buffer.
  std::size_t bindVertexBufferImpl(Uint32 slot, SDL_GPUBuffer *buffer,
                                   Uint32 offset);

  MeshLayout _layout;

public:
  /// Either the vertex count or, for indexed meshes, the index count;
  Uint32 count = ~Uint32{};
  // Having the vertex count is always useful, e.g. for batched ops.
  Uint32 vertexCount;

  std::vector<SDL_GPUBuffer *> vertexBuffers;
  std::vector<Uint32> vertexBufferOffsets;
  std::vector<BufferOwnership> vertexBufferOwnerships;
  SDL_GPUBuffer *indexBuffer{NULL};
  BufferOwnership indexBufferOwnership;
  Uint32 indexBufferOffset{0};
};

} // namespace candlewick
