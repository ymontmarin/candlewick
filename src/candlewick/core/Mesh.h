#pragma once

#include "Core.h"
#include "MeshLayout.h"

namespace candlewick {

/// \brief Convenience class for handling meshes: manages vertex and index
/// buffers.
struct Mesh {
  explicit Mesh(MeshLayout layout);

  Mesh(const Mesh &) = delete;
  Mesh(Mesh &&) noexcept = default;

  Mesh &operator=(const Mesh &) = delete;
  Mesh &operator=(Mesh &&) noexcept = delete;

  const MeshLayout &layout() const { return _layout; }

  Mesh &addVertexBuffer(Uint32 slot, SDL_GPUBuffer *buffer, Uint64 offset);
  Mesh &setIndexBuffer(SDL_GPUBuffer *buffer, Uint64 offset);

  bool isIndexed() const { return indexBuffer; }

  void releaseBuffers(const Device &device);

private:
  /// Add a vertex buffer corresponding to binding slot @p binding,
  /// using the pre-allocated @p buffer.
  std::size_t addVertexBufferImpl(Uint32 slot, SDL_GPUBuffer *buffer,
                                  Uint64 offset);

  MeshLayout _layout;

public:
  std::vector<SDL_GPUBuffer *> vertexBuffers;
  std::vector<Uint64> vertexBufferOffsets;
  SDL_GPUBuffer *indexBuffer;
  Uint64 indexBufferOffset;
};

} // namespace candlewick
