#pragma once

#include "../core/math_types.h"
#include <SDL3/SDL_stdinc.h>

namespace candlewick {

struct Mesh;

struct MeshData {
  std::vector<Float3> vertices;
  std::vector<Vec3u> faces;
  std::vector<Float3> normals;

  Uint32 numVertices() const { return static_cast<Uint32>(vertices.size()); }

  constexpr explicit MeshData() = default;
  MeshData(const MeshData &) = delete;
  MeshData(MeshData &&) noexcept = default;
  MeshData &operator=(MeshData &&) noexcept = default;
};

Mesh convertToMesh(const MeshData &meshData);

} // namespace candlewick
