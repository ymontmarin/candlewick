#pragma once

#include "candlewick/core/math_types.h"

namespace candlewick {

/// \name Camera control functions.
/// These functions are prefixed `camera` but take the view matrix and
/// not the camera's pose matrix.
/// \{

inline Float3 cameraViewPos(const Eigen::Matrix4f &viewMatrix) {
  auto R = viewMatrix.topLeftCorner<3, 3>();
  auto translation = viewMatrix.topRightCorner<3, 1>();
  return -R.transpose() * translation;
}

inline void cameraLocalRotateX(Eigen::Matrix4f &viewMatrix, Rad<float> angle) {
  float c, s;
  c = std::cos(angle);
  s = std::sin(angle);
  Eigen::Matrix3f R;
  R << 1., 0, 0, //
      0, c, -s,  //
      0, s, +c;
  viewMatrix.topLeftCorner<3, 3>().applyOnTheLeft(R);
}

/// \brief Rotate the camera around the center by a given increment.
inline void cameraWorldRotateZ(Eigen::Matrix4f &viewMatrix, Rad<float> angle) {
  float c, s;
  c = std::cos(angle);
  s = std::sin(angle);
  Eigen::Matrix3f R;
  R << c, -s, 0., //
      s, +c, 0.,  //
      0., 0., 1.;
  viewMatrix.topLeftCorner<3, 3>().applyOnTheRight(R);
}

inline void cameraLocalTranslate(Eigen::Matrix4f &viewMatrix,
                                 const Float3 &tr) {
  viewMatrix.topRightCorner<3, 1>() += tr;
}

inline void cameraLocalTranslateX(Eigen::Matrix4f &viewMatrix, float step) {
  cameraLocalTranslate(viewMatrix, {step, 0., 0.});
}

inline void cameraWorldTranslate(Eigen::Matrix4f &viewMatrix,
                                 const Float3 &tr) {
  auto R = viewMatrix.topLeftCorner<3, 3>();
  auto p = viewMatrix.topRightCorner<3, 1>();
  p += R * tr;
}

inline void cameraWorldTranslateX(Eigen::Matrix4f &viewMatrix, float step) {
  cameraWorldTranslate(viewMatrix, {step, 0., 0.});
}

inline void cameraWorldTranslateY(Eigen::Matrix4f &viewMatrix, float step) {
  cameraWorldTranslate(viewMatrix, {0., step, 0.});
}

inline void cameraWorldTranslateZ(Eigen::Matrix4f &viewMatrix, float step) {
  cameraWorldTranslate(viewMatrix, {0., 0., step});
}

/// \}

/// \name Cylindrical camera controls.
/// \{

inline void cylinderCameraViewportDrag(Eigen::Matrix4f &viewMatrix, Float2 step,
                                       float rotSensitivity,
                                       float panSensitivity,
                                       bool yinvert = true) {
  step.x() *= rotSensitivity;
  step.y() *= panSensitivity;
  float ystep = yinvert ? -step.y() : step.y();
  cameraWorldTranslateZ(viewMatrix, ystep);
  cameraWorldRotateZ(viewMatrix, Rad(step.x()));
}

inline void cylinderCameraMoveInOut(Eigen::Matrix4f &viewMatrix, float scale,
                                    float offset) {
  const float alpha = 1.f - (offset > 0 ? 1.f / scale : scale);
  const float curDist = viewMatrix.topRightCorner<3, 1>().norm();
  viewMatrix(2, 3) += curDist * alpha;
}

/// \}

} // namespace candlewick
