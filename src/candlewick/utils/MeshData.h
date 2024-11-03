#pragma once

#include "../core/math_types.h"
#include <SDL3/SDL_stdinc.h>

namespace candlewick {

struct Mesh;

struct MeshData {
  // use a vertex struct, allows us to interleave data properly
  struct Vertex {
    Float3 pos;
    Float3 normal;
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

Mesh convertToMesh(const MeshData &meshData);

} // namespace candlewick
