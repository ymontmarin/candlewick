#pragma once

#include "candlewick/core/math_types.h"

#include <Eigen/Geometry>

namespace candlewick {

/// \brief Rotate the camera around the center by a given increment.
inline void cylinderCameraZRotate(Eigen::Matrix4f &viewMatrix, float step,
                                  Float2 center) {
  // transform to compose with view matrix
  Eigen::Affine3f camTransform = Eigen::Affine3f::Identity();
  camTransform.translate(Float3{-center.x(), -center.y(), 0.});
  camTransform.rotate(Eigen::AngleAxisf{step, Float3::UnitZ()});
  camTransform.translate(Float3{center.x(), center.y(), 0.});

  viewMatrix.applyOnTheRight(camTransform.matrix());
}

inline void cylinderCameraUpDown(Eigen::Matrix4f &viewMatrix, float step) {
  Eigen::Affine3f transform = Eigen::Affine3f({0., 0., step});
  viewMatrix.applyOnTheRight(transform.matrix());
}

inline void cylinderCameraZoom(Eigen::Matrix4f &viewMatrix, float yoff) {
  float scale = 0.85f;
  Eigen::Ref<Float3> camPos{viewMatrix.col(3).head<3>()};
  Float3 fwd{viewMatrix.col(2).head<3>()};
  const float length = camPos.norm();
  const float alpha = 1.0f - (yoff > 0 ? 1 / scale : scale);

  auto zAxis = Eigen::Vector3f::UnitZ();
  camPos += zAxis * length * alpha;
}

} // namespace candlewick
