#include "Cone.h"
#include "Internal.h"
#include "../core/DefaultVertex.h"

namespace candlewick {

MeshData loadCone(Uint32 segments, float length) {
  detail::ConeCylinderBuilder builder;
  detail::builderAddCone(builder, segments, length);

  std::vector<DefaultVertex> vertices;
  vertices.resize(builder.currentVertices());
  for (std::size_t i = 0; i < builder.currentVertices(); i++) {
    vertices[i] = {builder.positions[i], builder.normals[i]};
  }
  return MeshData{SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, vertices,
                  builder.indices};
}

} // namespace candlewick
