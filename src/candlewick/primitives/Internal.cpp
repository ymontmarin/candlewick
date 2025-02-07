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
    const Uint32 base_centre_idx = Uint32(currentVertices());

    // BOTTOM
    addDisk(segments, radius, zBottom);

    // CONE TIP
    // direction from (radius, 0) in the xz-plane to the tip at (0, length).
    // normal vector to updir points outward towards (x+,z+)
    Uint32 current_floor_start_idx = base_centre_idx + 1;
    Float2 base_point{radius, zBottom};
    const Float2 updir{-radius, length};
    const Float2 base_normal = Float2{length, radius}.normalized();

    for (size_t i = 0; i < 2; i++) {

      Uint32 next_floor_start_idx = Uint32(currentVertices());
      for (Uint32 i = 0; i < segments; i++) {
        float a = static_cast<float>(i) * angleIncrement;
        float c = cosf(a);
        float s = sinf(a);
        Float3 pos{c * base_point[0], s * base_point[0], base_point[1]};
        add(pos, {c * base_normal[0], s * base_normal[0], base_normal[1]});
      }
      for (Uint32 i = 0; i < segments; i++) {
        const Uint32 offset = current_floor_start_idx;
        Uint32 j = i + offset;
        addFace({
            j,
            (i != segments - 1) ? j + 1 : offset,
            next_floor_start_idx + i,
        });
      }

      current_floor_start_idx = next_floor_start_idx;

      base_point += updir;
    }
  }
} // namespace detail
} // namespace candlewick
