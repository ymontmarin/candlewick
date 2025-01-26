#pragma once

#include "math_types.h"
#include "AABB.h"

namespace candlewick {

/// \brief Very simple oriented bounding-box (%OBB) class.
struct OBB {
  Mat3f axes;
  Float3 center;
  /// Half-sizes along the axes. This is consistent with COAL.
  Float3 halfExtents;

  static OBB fromAABB(const AABB &aabb) {
    OBB obb;
    obb.axes = Mat3f::Identity();
    obb.center = aabb.center();
    obb.halfExtents = 0.5f * aabb.extents();
    return obb;
  }

  void transformInPlace(const Mat4f &transform) {
    auto R = transform.topLeftCorner<3, 3>();
    auto tr = transform.topRightCorner<3, 1>();
    axes.applyOnTheLeft(R);
    center.applyOnTheLeft(R);
    center += tr;
  }

  /// Assuming rigid transform matrix.
  OBB transform(const Mat4f &transform) const {
    OBB res;
    auto R = transform.topLeftCorner<3, 3>();
    auto tr = transform.topRightCorner<3, 1>();
    res.axes.noalias() = R * axes;
    res.center.noalias() = R * center + tr;
    res.halfExtents = halfExtents;
    return res;
  }

  AABB toAabb() const {
    Float3 min = center, max = center;
    for (long i = 0; i < 3; i++) {
      Float3 axis = halfExtents[i] * axes.col(i);
      min -= axis.cwiseAbs();
      max += axis.cwiseAbs();
    }
    return AABB{min, max};
  }

  /// \brief Get a scale and translation matrix which transforms the NDC [-1,1]
  /// cube to the OBB.
  Mat4f toTransformationMatrix() const;
};

inline Mat4f OBB::toTransformationMatrix() const {
  Mat4f transform = Mat4f::Identity();
  auto D = halfExtents.asDiagonal();

  transform.block<3, 3>(0, 0) = axes * D;
  transform.block<3, 1>(0, 3) = center;
  return transform;
}

inline FrustumCornersType obbToCorners(const OBB &obb) {
  Mat4f tr = obb.toTransformationMatrix();
  FrustumCornersType out;
  for (Uint8 i = 0; i < 8; i++) {
    Uint8 j = i & 1;
    Uint8 k = (i >> 1) & 1;
    Uint8 l = (i >> 2) & 1;
    assert(j <= 1);
    assert(k <= 1);
    assert(l <= 1);

    Float4 ndc{2.f * j - 1.f, 2.f * k - 1.f, 2.f * l - 1.f, 1.f};
    Float4 viewSpace = tr * ndc;
    out[i] = viewSpace.head<3>() / viewSpace.w();
  }
  return out;
}

inline std::ostream &operator<<(std::ostream &oss, const OBB &obb) {
  FrustumCornersType corners = obbToCorners(obb);
  oss << "Corners {\n";
  for (Uint8 i = 0; i < 8; i++) {
    oss << "  [" << std::to_string(i) << "] = " << corners[i].transpose()
        << "\n";
  }
  return oss << "}";
}

} // namespace candlewick
