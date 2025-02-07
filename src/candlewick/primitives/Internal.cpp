#include "Internal.h"

namespace candlewick {
namespace detail {

  void builderAddCone(ConeCylinderBuilder &builder, Uint32 segments,
                      float length) {
    const float halfLength = 0.5f * length;
    const float angleIncrement = 2.f * constants::Pif / float(segments);

    // BOTTOM

    // bottom center vertex
    builder.add({0.f, 0.f, -halfLength}, {0.f, 0.f, -1.f});
    // bottom ring
    for (Uint32 i = 0; i < segments; ++i) {
      float a = static_cast<float>(i) * angleIncrement;
      float x = cosf(a);
      float y = sinf(a);

      builder.add({x, y, -halfLength}, {x * length, y * length, -1.f});
    }

    // lace up the bottom disk (define triangle indices)
    Uint32 cap_idx = 0u;
    for (Uint32 i = cap_idx + 1; i < segments; ++i) {
      builder.addFace({i, i + 1, cap_idx});
    }

    // CONE SHAFT

    // cone tip
    builder.add({0.f, 0.f, halfLength}, {0.f, 0.f, 1.f});

    // lace up towards the tip
    Uint32 tip_idx = Uint32(builder.positions.size() - 1);
    assert(tip_idx == segments + 1);
    for (Uint32 i = cap_idx + 1; i < tip_idx - 1; i++) {
      builder.addFace({i, i + 1, tip_idx});
    }
  }
} // namespace detail
} // namespace candlewick
