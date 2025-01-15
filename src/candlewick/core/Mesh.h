#pragma once

#include "Core.h"
#include "Tags.h"
#include "MeshLayout.h"
#include <vector>

namespace candlewick {

/// \brief Handle class for meshes (vertex buffers and an optional index buffer)
/// on the GPU.

/// This class contains the layout, vertex (and index) count(s), and handles to
/// the GPU vertex and index buffers the \c Mesh references. A \c Mesh may be
/// just a view into these buffers: this is controlled using the \c
/// BufferOwnership flag and the vertex and index buffer offsets.
struct Mesh {
  enum BufferOwnership {
    /// The buffer is owned, and will be released when releaseOwnedBuffers() is
    /// called.
    Owned,
    /// The buffer is borrowed. Some other object (or the user) is responsible
    /// for releasing it.
    Borrowed,
  };

  explicit Mesh(MeshLayout layout);

  Mesh(NoInitT);
  Mesh(const Mesh &) = delete;
  Mesh(Mesh &&) noexcept = default;

  Mesh &operator=(const Mesh &) = delete;
  Mesh &operator=(Mesh &&) noexcept = default;

  const MeshLayout &layout() const { return _layout; }

  /// \brief Bind an existing vertex buffer to a given slot of the \c Mesh.
  ///
  /// \param slot Binding slot of the vertex buffer. Used by
  /// SDL_GPUVertexAttribute.
  /// \param buffer Existing buffer.
  /// \param offset Desired offset for the \c Mesh in the vertex buffer. This
  /// sets the corresponding entry in vertexBufferOffsets.
  /// \param ownershop Whether to borrow or take ownership of the given buffer.
  Mesh &bindVertexBuffer(Uint32 slot, SDL_GPUBuffer *buffer, Uint32 offset,
                         BufferOwnership ownership = Borrowed);

  /// \brief Bind an existing index buffer for the \c Mesh.
  ///
  /// \param buffer Existing index buffer.
  /// \param offset Desired offset for the \c Mesh in the index buffer. This
  /// sets the value of indexBufferOffset.
  /// \param ownershop Whether to borrow or take ownership of the given buffer.
  Mesh &setIndexBuffer(SDL_GPUBuffer *buffer, Uint32 offset,
                       BufferOwnership ownership = Borrowed);

  bool isIndexed() const { return indexBuffer != NULL; }
  bool isCountSet() const { return count != ~Uint32{}; }

  /// \brief Release all owned vertex and index buffers in the \c Mesh object.
  /// \param device Handle to the GPU device.
  void releaseOwnedBuffers(SDL_GPUDevice *device);

  SDL_GPUBufferBinding getVertexBinding(Uint32 slot) const {
    return {.buffer = vertexBuffers[slot], .offset = vertexBufferOffsets[slot]};
  }

  SDL_GPUBufferBinding getIndexBinding() const {
    return {.buffer = indexBuffer, .offset = indexBufferOffset};
  }

private:
  /// Add a vertex buffer corresponding to binding slot \param slot,
  /// using the pre-allocated \param buffer.
  std::size_t bindVertexBufferImpl(Uint32 slot, SDL_GPUBuffer *buffer,
                                   Uint32 offset);

  MeshLayout _layout;

public:
  /// Either the vertex count or, for indexed meshes, the index count.
  Uint32 count = ~Uint32{};
  /// Having the vertex count is always useful, e.g. for batched ops.
  Uint32 vertexCount;

  /// Vertex buffers the \c Mesh has its vertex data in.
  ///
  /// There may be multiple vertex buffers, depending on how the mesh data is
  /// laid out. For instance, some vertex attributes may be non-interleaved,
  /// e.g. the vertex positions, normals, and colors may be in different vertex
  /// buffers in GPU memory, instead of being laid out as
  /// `[pos0, norm0, col0, pos1, ...]`.
  std::vector<SDL_GPUBuffer *> vertexBuffers;
  /// \brief Offset (starting byte) of the \c Mesh object inside each vertex
  /// buffer. Useful for Meshes which share vertex buffers with others.
  std::vector<Uint32> vertexBufferOffsets;
  /// Flag for whether each vertex buffer is owned.
  std::vector<BufferOwnership> vertexBufferOwnerships;
  /// Index buffer the \c Mesh has its index data in. If this is NULL, then the
  /// \c Mesh is considered to be *non*-indexed.
  SDL_GPUBuffer *indexBuffer{NULL};
  /// Flag for whether the index buffer is owned.
  BufferOwnership indexBufferOwnership;
  /// \brief Offset (starting byte) of the \c Mesh object inside the index
  /// buffer. Useful for Meshes which share vertex buffers with others.
  Uint32 indexBufferOffset{0};
};

} // namespace candlewick
