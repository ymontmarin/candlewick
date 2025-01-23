#pragma once

#include "math_types.h"
#include "AABB.h"

namespace candlewick {

/// \brief Very simple oriented bounding-box (%OBB) class.
struct OBB {
  Mat3f axes;
  Float3 center;
  Float3 extents;

  static OBB fromAABB(const AABB &aabb) {
    OBB obb;
    obb.axes = Mat3f::Identity();
    obb.center = aabb.center();
    obb.extents = aabb.extents();
    return obb;
  }

  /// Assuming rigid transform matrix.
  OBB transform(const Mat4f &transform) const {
    OBB res;
    auto R = transform.topLeftCorner<3, 3>();
    auto tr = transform.topRightCorner<3, 1>();
    res.axes.noalias() = R * axes;
    res.center.noalias() = R * center + tr;
    res.extents = extents;
    return res;
  }

  AABB toAabb() const {
    Float3 min = center, max = center;
    for (long i = 0; i < 3; i++) {
      Float3 axis = extents[i] * axes.col(i);
      min -= axis.cwiseAbs();
      max += axis.cwiseAbs();
    }
    return AABB{min, max};
  }
};

} // namespace candlewick
