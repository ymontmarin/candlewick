#include "Cone.h"
#include "Internal.h"
#include "../core/DefaultVertex.h"

namespace candlewick {

MeshData loadCone(Uint32 segments, float radius, float length) {
  detail::ConeCylinderBuilder builder;
  const float halfLength = 0.5f * length;
  builder.addCone(segments, radius, -halfLength, length);

  std::vector<DefaultVertex> vertices;
  vertices.resize(builder.currentVertices());
  for (std::size_t i = 0; i < builder.currentVertices(); i++) {
    vertices[i] = {builder.positions[i], builder.normals[i]};
  }
  return MeshData{SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, vertices,
                  builder.indices};
}

} // namespace candlewick
