#pragma once

#include "candlewick/core/math_types.h"

#include <Eigen/Geometry>

namespace candlewick {

/// \brief Rotate the camera around the center by a given increment.
inline void cylinderCameraZRotate(Eigen::Matrix4f &viewMatrix, Rad<float> step,
                                  Float2 center) {
  // transform to compose with view matrix
  Eigen::Affine3f camTransform = Eigen::Affine3f::Identity();
  camTransform.translate(Float3{-center.x(), -center.y(), 0.});
  camTransform.rotate(Eigen::AngleAxisf{step, Float3::UnitZ()});
  camTransform.translate(Float3{center.x(), center.y(), 0.});

  viewMatrix.applyOnTheRight(camTransform.matrix());
}

inline void cylinderXLocalRotate(Eigen::Matrix4f &viewMatrix,
                                 Rad<float> angle) {
  Eigen::AngleAxisf aa{angle, Eigen::Vector3f::UnitX()};
  Eigen::Affine3f tr{aa};
  Float3 center = viewMatrix.col(3).head<3>();
  viewMatrix.col(3).head<3>().setZero();
  viewMatrix.applyOnTheLeft(tr.matrix());
  viewMatrix.col(3).head<3>() = center;
}

inline void cylinderCameraUpDown(Eigen::Matrix4f &viewMatrix, float step) {
  Eigen::Affine3f transform = Eigen::Affine3f({0., 0., step});
  viewMatrix.applyOnTheRight(transform.matrix());
}

inline void cylinderCameraViewportDrag(Eigen::Matrix4f &viewMatrix, Float2 step,
                                       bool yinvert = true) {
  float ystep = yinvert ? -step.y() : step.y();
  cylinderCameraUpDown(viewMatrix, ystep);
  cylinderCameraZRotate(viewMatrix, Rad(step.x()), {0., 0.});
}

} // namespace candlewick
