#pragma once

#include "../core/Core.h"
#include "../core/math_types.h"
#include <SDL3/SDL_gpu.h>

namespace candlewick {

struct Mesh;

struct MeshData {
  // use a vertex struct, allows us to interleave data properly
  struct Vertex {
    Float3 pos;
    alignas(16) Float3 normal;
  };
  using IndexType = Uint32;
  std::vector<Vertex> vertexData;
  std::vector<IndexType> indexData;

  std::size_t numVertices() const { return vertexData.size(); }
  std::size_t numIndices() const { return indexData.size(); }

  constexpr explicit MeshData() = default;
  MeshData(const MeshData &) = delete;
  MeshData(MeshData &&) noexcept = default;
  MeshData &operator=(MeshData &&) noexcept = default;
  MeshData(std::vector<Vertex> vertexData, std::vector<IndexType> indexData);
};

/// \brief Convert @c MeshData to a triangle @c Mesh. This creates the required
/// vertex buffer.
Mesh convertToMesh(const Device &device, const MeshData &meshData);

Mesh convertToMeshIndexed(const MeshData &meshData, SDL_GPUBuffer *vertexBuffer,
                          Uint64 vertexOffset, SDL_GPUBuffer *indexBuffer,
                          Uint64 indexOffset, bool takeOwnership = false);

/// \brief Convert @c MeshData to an indexed triangle @c Mesh. This creates the
/// required vertex and index buffers.
Mesh convertToMeshIndexed(const Device &device, const MeshData &meshData);

/// \brief Upload mesh contents to GPU device.
void uploadMeshToDevice(const Device &device, const Mesh &mesh,
                        const MeshData &meshData);
} // namespace candlewick
