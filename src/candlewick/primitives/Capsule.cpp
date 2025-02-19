#include "Capsule.h"
#include "Internal.h"
#include "../core/DefaultVertex.h"

namespace candlewick {

MeshData loadCapsuleSolid(Uint32 hemisphereRings, Uint32 segments,
                          float length) {
  using constants::Pi_2f;
  using constants::Pif;
  detail::ConeCylinderBuilder builder;
  hemisphereRings = std::max(2u, hemisphereRings);
  segments = std::max(3u, segments);
  assert(length >= 0.0f);

  const float halfLength = 0.5f * length;

  const Radf ringIncrement{Pif / float(hemisphereRings)};

  // Bottom cap
  builder.add({0.f, 0.f, -1.f - halfLength}, {0.f, 0.f, -1.f});
  for (Uint32 j = 0; j < segments; j++) {
    builder.addFace({
        0u,
        (j != segments - 1) ? j + 2 : 1,
        j + 1,
    });
  }

  // Bottom hemisphere
  const Radf ringStart = Radf(ringIncrement - Pi_2f);
  builder.addHemisphereVertices(hemisphereRings - 1u, segments,
                                -halfLength,   // start from -l/2
                                ringStart,     // Start from equator
                                ringIncrement, // Move downwards
                                1u             // Start from beginning
  );

  // Cylinder middle section
  // Connects with hemispheres at their edges
  Float2 basePoint{1.f, -halfLength}; // start circle in XY plane
  Float2 upDir{0.0f, length};         // up direction for circle

  builder.addCylinderFloors(2u, segments, basePoint, upDir,
                            builder.currentVertices());

  // Top hemisphere
  builder.addHemisphereVertices(hemisphereRings - 1u, segments,
                                halfLength,               // start at l/2
                                Radf(0.f),                // Start from equator
                                ringIncrement,            // Move upwards
                                builder.currentVertices() // continue on
  );

  // Top cap
  builder.add({0.f, 0.f, 1.f + halfLength}, {0.f, 0.f, 1.f});
  const Uint32 currentCount = builder.currentVertices();
  const Uint32 startIdxTop = currentCount - segments - 1u;
  for (Uint32 i = 0; i < segments; i++) {
    const Uint32 j = startIdxTop + i;
    builder.addFace({
        j,
        (i != segments - 1) ? j + 1 : startIdxTop,
        currentCount - 1u,
    });
  }

  std::vector<DefaultVertex> vertices;
  vertices.reserve(builder.currentVertices());
  for (size_t i = 0; i < builder.currentVertices(); i++) {
    DefaultVertex v;
    v.pos = builder.positions[i];
    v.normal = builder.normals[i];
    vertices.push_back(v);
  }
  return MeshData{SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, std::move(vertices),
                  std::move(builder.indices)};
}

} // namespace candlewick
