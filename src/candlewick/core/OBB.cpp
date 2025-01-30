#include "OBB.h"

#include "AABB.h"

namespace candlewick {

OBB OBB::fromAABB(const AABB &aabb) {
  OBB obb;
  obb.axes = Mat3f::Identity();
  obb.center = aabb.center();
  obb.halfExtents = 0.5f * aabb.extents();
  return obb;
}

AABB OBB::toAabb() const {
  Float3 min = center, max = center;
  for (long i = 0; i < 3; i++) {
    Float3 axis = halfExtents[i] * axes.col(i);
    min -= axis.cwiseAbs();
    max += axis.cwiseAbs();
  }
  return AABB{min, max};
}

} // namespace candlewick
