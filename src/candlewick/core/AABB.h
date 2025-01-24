#pragma once

#include "math_types.h"
#include "Tags.h"

namespace candlewick {

struct AABB {
  Float3 min;
  Float3 max;

  AABB(NoInitT) {}
  AABB()
      : min(Float3::Constant(std::numeric_limits<float>::max())),
        max(Float3::Constant(std::numeric_limits<float>::lowest())) {}
  AABB(const Float3 &v) : min(v), max(v) {}
  AABB(const Float3 &a, const Float3 &b)
      : min(a.cwiseMin(b)), max(a.cwiseMax(b)) {}

  static AABB fromPoints(const Float3 &a, const Float3 &b, const Float3 &c) {
    AABB out{NoInit};
    out.min = a.cwiseMin(b).cwiseMin(c);
    out.max = a.cwiseMax(b).cwiseMax(c);
    return out;
  }

  /// Box sizes along the axes.
  Float3 extents() const { return max - min; }
  /// Compute the box's center point.
  Float3 center() const { return 0.5f * (min + max); }

  float width() const { return max.x() - min.x(); }
  float height() const { return max.y() - min.y(); }
  float depth() const { return max.z() - min.z(); }
  float volume() const { return width() * height() * depth(); }

  AABB &grow(const Float3 &delta) {
    assert(delta.minCoeff() >= 0);
    min -= delta;
    max += delta;
    return *this;
  }

  AABB &grow(float delta) {
    assert(delta >= 0);
    min.array() -= delta;
    max.array() += delta;
    return *this;
  }

  AABB &grow(const AABB &other) {
    min = min.cwiseMin(other.min);
    max = max.cwiseMax(other.max);
    return *this;
  }

  // Compute the AABB bounding box of the set union of \c this and \p other.
  AABB reunion(const AABB &other) {
    AABB ret{*this};
    return ret.grow(other);
  }

  /// \brief Produce array of 3D corners of the bounding box.
  ///
  /// Useful for producing a camera frustum.
  std::array<Float3, 8> corners() const {
    std::array<Float3, 8> out;
    const Float3 ab[2] = {min, max};
    for (uint8_t i = 0; i < 8; i++) {
      uint8_t j = i & 1;
      uint8_t k = (i >> 1) & 1;
      uint8_t l = (i >> 2) & 1;

      out[i] = Float3{ab[j].x(), ab[k].y(), ab[l].z()};
    }
    return out;
  }
};

} // namespace candlewick
