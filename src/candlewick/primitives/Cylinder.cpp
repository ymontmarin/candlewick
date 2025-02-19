#include "Cylinder.h"
#include "Internal.h"
#include "../core/DefaultVertex.h"

namespace candlewick {

MeshData loadCylinderSolid(Uint32 rings, Uint32 segments, float radius,
                           float height) {
  detail::ConeCylinderBuilder builder;
  const float halfLength = 0.5f * height;
  const float z = -halfLength;

  builder.addBottomDisk(segments, radius, z);
  assert(rings >= 1);
  const Float2 upDir{0., height};
  builder.addCylinderFloors(rings, segments, {radius, z}, upDir, 1);

  builder.addTopDisk(segments, radius, halfLength);

  std::vector<DefaultVertex> vertices;
  vertices.resize(builder.currentVertices());
  for (std::size_t i = 0; i < builder.currentVertices(); i++) {
    vertices[i] = {builder.positions[i], builder.normals[i]};
  }
  return MeshData{
      SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
      std::move(vertices),
      std::move(builder.indices),
  };
}

} // namespace candlewick
