#include "Internal.h"

namespace candlewick {
namespace detail {
  void ConeCylinderBuilder::addBottomDisk(Uint32 segments, float radius,
                                          float z) {
    const float angleIncrement = 2.f * constants::Pif / float(segments);

    Uint32 cap_idx = currentVertices();
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

  void ConeCylinderBuilder::addTopDisk(Uint32 segments, float radius, float z) {
    const float angleIncrement = 2.f * constants::Pif / float(segments);

    for (Uint32 i = 0; i < segments; i++) {
      float a = float(i) * angleIncrement;
      float x = cosf(a) * radius;
      float y = sinf(a) * radius;
      add({x, y, z}, {0.f, 0.f, +1.f});
    }

    add({0, 0, z}, {0.f, 0.f, +1.f});

    Uint32 currentCount = currentVertices();

    // lace up the top disk
    Uint32 startIdx = currentCount - segments - 1u;
    for (Uint32 i = 0; i < segments; ++i) {
      const Uint32 j = startIdx + i;
      addFace({
          j,
          (i != segments - 1) ? j + 1 : startIdx,
          // top vertex
          currentCount - 1,
      });
    }
  }

  void ConeCylinderBuilder::addCylinderFloors(Uint32 numFloors, Uint32 segments,
                                              Float2 basePoint, Float2 upDir,
                                              Uint32 startIdx) {
    upDir /= float(numFloors - 1);
    const float angleIncrement =
        2.f * constants::Pif / static_cast<float>(segments);
    const Float2 baseNormal = Float2{upDir.y(), -upDir.x()}.normalized();

    for (size_t i = 0; i < numFloors; i++) {

      Uint32 next_floor_start_idx = currentVertices();
      for (Uint32 j = 0; j < segments; j++) {
        float a = static_cast<float>(j) * angleIncrement;
        float c = cosf(a);
        float s = sinf(a);
        Float3 pos{c * basePoint[0], s * basePoint[0], basePoint[1]};
        add(pos, {c * baseNormal[0], s * baseNormal[0], baseNormal[1]});
      }
      for (Uint32 j = 0; j < segments; j++) {
        Uint32 bl = j + startIdx;
        Uint32 br = (j != segments - 1) ? bl + 1 : startIdx;
        Uint32 tl = j + next_floor_start_idx;
        Uint32 tr = (j != segments - 1) ? tl + 1 : next_floor_start_idx;
        addFace({
            bl,
            br,
            tl,
        });
        addFace({
            tl,
            br,
            tr,
        });
      }
      startIdx = next_floor_start_idx;
      basePoint += upDir;
    }
  }

  void ConeCylinderBuilder::addCone(Uint32 segments, float radius,
                                    float zBottom, float length) {
    const Uint32 base_centre_idx = Uint32(currentVertices());

    // BOTTOM
    addBottomDisk(segments, radius, zBottom);

    // CONE TIP
    // direction from (radius, 0) in the xz-plane to the tip at (0, length).
    // normal vector to updir points outward towards (x+,z+)
    Uint32 current_floor_start_idx = base_centre_idx + 1;
    Float2 basePoint{radius, zBottom};
    const Float2 upDir{-radius, length};
    addCylinderFloors(2, segments, basePoint, upDir, current_floor_start_idx);
  }

  void ConeCylinderBuilder::addHemisphereVertices(Uint32 count, Uint32 segments,
                                                  float zCenter, Radf ringStart,
                                                  float ringIncrement,
                                                  Uint32 startIdx) {
    const float segmentIncrement =
        2.f * constants::Pif / static_cast<float>(segments);

    for (Uint32 i = 0; i < count; i++) {
      const float ringAngle = ringStart + float(i) * ringIncrement;
      const float x = cosf(ringAngle);
      const float y = x;
      const float z = sinf(ringAngle);

      Uint32 next_floor_start_idx = currentVertices();
      for (Uint32 j = 0; j < segments; j++) {
        const float segAngle = float(j) * segmentIncrement;
        const float c = cosf(segAngle);
        const float s = -sinf(segAngle);
        add({x * s, y * c, z + zCenter}, {x * s, y * c, z});
      }
      for (Uint32 j = 0; j < segments; j++) {
        Uint32 bl = j + startIdx;
        Uint32 br = (j != segments - 1) ? bl + 1 : startIdx;
        Uint32 tl = j + next_floor_start_idx;
        Uint32 tr = (j != segments - 1) ? tl + 1 : next_floor_start_idx;
        addFace({
            bl,
            br,
            tl,
        });
        addFace({
            tl,
            br,
            tr,
        });
      }

      startIdx = next_floor_start_idx;
    }
  }
} // namespace detail
} // namespace candlewick
