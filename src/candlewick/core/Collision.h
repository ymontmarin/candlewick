#pragma once
#include "math_types.h"
#include <coal/BV/AABB.h>
#include <coal/BV/OBB.h>

namespace candlewick {

using coal::AABB;
using coal::OBB;

inline Mat4f toTransformationMatrix(const AABB &aabb) {
  Mat4f T = Mat4f::Identity();
  Float3 halfExtents = 0.5f * (aabb.max_ - aabb.min_).cast<float>();
  T.block<3, 3>(0, 0) = halfExtents.asDiagonal();
  T.topRightCorner<3, 1>() = aabb.center().cast<float>();
  return T;
}

inline Mat4f toTransformationMatrix(const OBB &obb) {
  Mat4f T = Mat4f::Identity();
  auto D = obb.extent.asDiagonal();
  T.block<3, 3>(0, 0) = (obb.axes * D).cast<float>();
  T.topRightCorner<3, 1>() = obb.center().cast<float>();
  return T;
}

} // namespace candlewick
