#include "Sphere.h"
#include "Internal.h"
#include "../core/DefaultVertex.h"

namespace candlewick {

MeshData loadUvSphereSolid(Uint32 rings, Uint32 segments) {
  float ringIncrement = constants::Pif / float(rings);
  detail::ConeCylinderBuilder builder;
  builder.add({0., 0., -1.f}, {0., 0., -1.f});
  // bottom ring
  for (Uint32 j = 0; j < segments; j++) {
    builder.addFace({0u, (j != segments - 1) ? j + 2 : 1, j + 1});
  }

  Radf ringStart = Radf(ringIncrement - constants::Pi_2f);
  builder.addHemisphereVertices(rings - 1, segments, 0.0f, ringStart,
                                ringIncrement, 1u);

  builder.add({0., 0., 1.f}, {0., 0., 1.f});
  Uint32 currentCount = builder.currentVertices();
  Uint32 startIdxTop = currentCount - segments - 1u;
  for (Uint32 i = 0; i < segments; i++) {
    const Uint32 j = startIdxTop + i;
    builder.addFace({
        j,
        // next vertex - loop back if j is last
        (i != segments - 1) ? j + 1 : startIdxTop,
        // top vertex - last index!
        currentCount - 1u,
    });
  }

  std::vector<DefaultVertex> vertices;
  for (Uint32 j = 0; j < builder.currentVertices(); j++) {
    vertices.push_back({builder.positions[j], builder.normals[j]});
  }
  MeshData out{
      SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
      std::move(vertices),
      std::move(builder.indices),
  };
  out.material.baseColor = 0xFFDA61ff_rgbaf;
  return out;
}

} // namespace candlewick
