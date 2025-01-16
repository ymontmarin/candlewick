#pragma once

#include "Core.h"
#include "Tags.h"
#include "MeshLayout.h"

#include <vector>
#include <SDL3/SDL_assert.h>

namespace candlewick {

/// \brief Handle class for meshes (vertex buffers and an optional index buffer)
/// on the GPU.
///
/// This class contains the layout, vertex (and index) count(s), and handles to
/// the GPU vertex and index buffers the Mesh references.
///
/// A Mesh **owns** its vertex and index buffers.
///
/// \sa MeshView
struct Mesh {
  MeshLayout layout;
  Uint32 vertexCount;
  Uint32 indexCount{0u};

  /// Vertex buffers the Mesh has its vertex data in.
  ///
  /// There may be multiple vertex buffers, depending on how the mesh data is
  /// laid out. For instance, some vertex attributes may be non-interleaved,
  /// e.g. the vertex positions, normals, and colors may be in different vertex
  /// buffers in GPU memory, instead of being laid out as
  /// `[pos0, norm0, col0, pos1, ...]`.
  std::vector<SDL_GPUBuffer *> vertexBuffers;

  /// Index buffer for the Mesh 's index data. If this is NULL, then the
  /// Mesh is considered to be *non*-indexed.
  SDL_GPUBuffer *indexBuffer{NULL};

  explicit Mesh(MeshLayout layout);

  Mesh(NoInitT);
  Mesh(const Mesh &) = delete;
  Mesh(Mesh &&) noexcept = default;

  Mesh &operator=(const Mesh &) = delete;
  Mesh &operator=(Mesh &&) noexcept = default;

  /// \brief Bind an existing vertex buffer to a given slot of the Mesh.
  /// \warning This function will **take ownership of the buffer**.
  ///
  /// \param slot Binding slot of the vertex buffer. Used by
  /// SDL_GPUVertexAttribute.
  /// \param buffer Existing buffer.
  Mesh &bindVertexBuffer(Uint32 slot, SDL_GPUBuffer *buffer);

  /// \brief Bind an existing index buffer for the Mesh.
  ///
  /// This function does **not** check that the buffer is non-null. The lack of
  /// check is to allow method chaining with arguments which may or may not be
  /// null, depending on context.
  /// \warning This function will **take ownership of the buffer**.
  ///
  /// \param buffer Existing index buffer.
  Mesh &setIndexBuffer(SDL_GPUBuffer *buffer);

  /// \brief Number of vertex buffers.
  Uint32 numVertexBuffers() const { return layout.numBuffers(); }

  bool isIndexed() const { return indexBuffer != NULL; }

  /// \brief Release all owned vertex and index buffers in the Mesh object.
  void release(SDL_GPUDevice *device);

  SDL_GPUBufferBinding getVertexBinding(Uint32 slot) const {
    return {.buffer = vertexBuffers[slot], .offset = 0u};
  }

  SDL_GPUBufferBinding getIndexBinding() const {
    return {.buffer = indexBuffer, .offset = 0u};
  }

  /// \brief Conversion operator to a MeshView object. This will produce a
  /// full-view of \c this.
  MeshView toView() const;
};

/// \brief A view into a Mesh object.
///
/// Objects of this class are copyable and movable, since they only "borrow" the
/// buffers.
///
/// \sa Mesh
struct MeshView {
  std::vector<SDL_GPUBuffer *> vertexBuffers;
  /// Vertex offsets, expressed in indices of the original vertices.
  std::vector<Uint32> vertexOffsets;
  Uint32 vertexCount;
  SDL_GPUBuffer *indexBuffer;
  /// Index offset (not in bytes).
  Uint32 indexOffset;
  Uint32 indexCount;

  bool isIndexed() const { return indexCount > 0; }
};

inline MeshView Mesh::toView() const {
  std::vector<Uint32> vtxOffsets;
  vtxOffsets.resize(numVertexBuffers(), 0u);
  return MeshView{
      .vertexBuffers = vertexBuffers,
      .vertexOffsets = std::move(vtxOffsets),
      .vertexCount = vertexCount,
      .indexBuffer = indexBuffer,
      .indexOffset = 0,
      .indexCount = indexCount,
  };
}

/// \brief Check that all vertex buffers were set, and consistency in the
/// "indexed/non-indexed" status.
[[nodiscard]] inline bool validateMesh(const Mesh &mesh) {
  for (size_t i = 0; i < mesh.numVertexBuffers(); i++) {
    if (!mesh.vertexBuffers[i])
      return false;
  }
  // check that if we are indexed iff we have positive index count.
  return mesh.isIndexed() == (mesh.indexCount > 0);
}

[[nodiscard]] inline bool validateMeshView(const MeshView &view) {
  for (auto vb : view.vertexBuffers) {
    if (!vb)
      return false;
  }
  if (!view.indexBuffer)
    return false;
  // views of indexed meshes cannot have zero indices.
  if (view.isIndexed() != (view.indexCount > 0))
    return false;
  return view.vertexCount >= 0;
}

} // namespace candlewick
