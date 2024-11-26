#pragma once

#include "candlewick/core/math_types.h"

namespace candlewick {

inline void cameraLocalXRotate(Eigen::Matrix4f &viewMatrix, Rad<float> angle) {
  float c, s;
  sincosf(angle, &s, &c);
  Eigen::Matrix3f R;
  R << 1., 0, 0, //
      0, c, -s,  //
      0, s, +c;
  viewMatrix.topLeftCorner<3, 3>().applyOnTheLeft(R);
}

/// \brief Rotate the camera around the center by a given increment.
inline void cylinderCameraZRotate(Eigen::Matrix4f &viewMatrix,
                                  Rad<float> angle) {
  float c, s;
  sincosf(angle, &s, &c);
  Eigen::Matrix3f R;
  R << c, -s, 0., //
      s, +c, 0.,  //
      0., 0., 1.;
  viewMatrix.topLeftCorner<3, 3>().applyOnTheRight(R);
}

inline void cylinderCameraUpDown(Eigen::Matrix4f &viewMatrix, float step) {
  Eigen::Matrix4f tr = Eigen::Matrix4f::Identity();
  tr(2, 3) = step;
  viewMatrix.applyOnTheRight(tr);
}

inline void cylinderCameraViewportDrag(Eigen::Matrix4f &viewMatrix, Float2 step,
                                       float rotSensitivity,
                                       float panSensitivity,
                                       bool yinvert = true) {
  step.x() *= rotSensitivity;
  step.y() *= panSensitivity;
  float ystep = yinvert ? -step.y() : step.y();
  cylinderCameraUpDown(viewMatrix, ystep);
  cylinderCameraZRotate(viewMatrix, Rad(step.x()));
}

} // namespace candlewick
