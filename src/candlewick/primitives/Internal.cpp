#include "Internal.h"

namespace candlewick {
namespace detail {

  void ConeCylinderBuilder::addCone(Uint32 segments, float radius,
                                    float zBottom, float length) {
    const float angleIncrement = 2.f * constants::Pif / float(segments);

    // BOTTOM

    // bottom center vertex
    Uint32 cap_idx = Uint32(currentVertices());
    add({0.f, 0.f, zBottom}, {0.f, 0.f, -1.f});
    // bottom ring
    for (Uint32 i = 0; i < segments; ++i) {
      float a = static_cast<float>(i) * angleIncrement;
      float x = cosf(a) * radius;
      float y = sinf(a) * radius;

      add({x, y, zBottom}, {x * length, y * length, -1.f});
    }

    // lace up the bottom disk (define triangle indices)
    for (Uint32 i = cap_idx; i < cap_idx + segments; ++i) {
      addFace({
          cap_idx,
          (i != cap_idx + segments - 1) ? i + 2 : 1u,
          i + 1,
      });
    }

    // CONE TIP

    add({0.f, 0.f, zBottom + length}, {0.f, 0.f, 1.f});
    Uint32 tip_idx = Uint32(currentVertices() - 1);
    for (Uint32 i = cap_idx + 1; i < tip_idx; i++) {
      addFace({
          i,
          (i != tip_idx - 1) ? i + 1 : 1u,
          tip_idx,
      });
    }
  }
} // namespace detail
} // namespace candlewick
