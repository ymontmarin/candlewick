#include "Internal.h"

namespace candlewick {
namespace detail {
  void ConeCylinderBuilder::addDisk(Uint32 segments, float radius, float z) {
    const float angleIncrement = 2.f * constants::Pif / float(segments);

    Uint32 cap_idx = Uint32(currentVertices());
    add({0.f, 0.f, z}, {0.f, 0.f, -1.f});
    // bottom ring
    for (Uint32 i = 0; i < segments; ++i) {
      float a = static_cast<float>(i) * angleIncrement;
      float x = cosf(a) * radius;
      float y = sinf(a) * radius;

      add({x, y, z}, {0.f, 0.f, -1.f});
    }

    // lace up the bottom disk (define triangle indices)
    for (Uint32 i = cap_idx; i < cap_idx + segments; ++i) {
      addFace({
          cap_idx,
          (i != cap_idx + segments - 1) ? i + 2 : 1u,
          i + 1,
      });
    }
  }

  void ConeCylinderBuilder::addCone(Uint32 segments, float radius,
                                    float zBottom, float length) {
    const float angleIncrement = 2.f * constants::Pif / float(segments);
    Uint32 base_centre_idx = Uint32(currentVertices());

    // BOTTOM
    addDisk(segments, radius, zBottom);

    // CONE TIP
    // direction from (radius, 0) in the xz-plane to the tip at (0, length).
    // normal vector to updir points outward towards (x+,z+)
    Float2 updir = {-radius, length};
    Float2 base_normal = Float2{length, radius}.normalized();
    Float3 tip_pos{0.f, 0.f, zBottom + length};

    Uint32 tip_start_idx = Uint32(currentVertices());
    for (Uint32 i = 0; i < segments; i++) {
      float a = static_cast<float>(i) * angleIncrement;
      float c = cosf(a);
      float s = sinf(a);
      add(tip_pos, {c * base_normal[0], s * base_normal[0], base_normal[1]});
    }
    for (Uint32 i = 0; i < segments; i++) {
      const Uint32 off = base_centre_idx + 1;
      Uint32 j = i + off;
      addFace({
          j,
          (i != segments - 1) ? j + 1 : off,
          tip_start_idx,
      });
    }
  }
} // namespace detail
} // namespace candlewick
