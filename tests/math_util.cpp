#include "math_util.hpp"

#include <Eigen/Geometry>

Eigen::Matrix4f lookAt(const Float3 &eye, const Float3 &center,
                       const Float3 &up) {
  Eigen::Matrix4f mat;
  mat.setIdentity();
  auto R = mat.block<3, 3>(0, 0);
  Float3 bwd = eye - center;
  bwd.normalize();
  Float3 y = up;
  Float3 x = y.cross(bwd).normalized();
  y = bwd.cross(x).normalized();
  R.col(0) = x;
  R.col(1) = y;
  R.col(2) = bwd;
  // coords of eye in new reference frame
  mat.col(3).head<3>() = -R * eye;
  return mat;
}

Eigen::Matrix4f perspectiveMatrix(float left, float right, float top,
                                  float bottom, float near, float far) {
  Eigen::Matrix4f out;
  float hfov = right - left;
  float vfov = top - bottom;
  out.setZero();
  out(0, 0) = 2 * near / hfov;
  out(0, 2) = (left + right) / hfov;

  out(1, 1) = 2 * near / vfov;
  out(1, 2) = (top + bottom) / vfov;

  out(2, 2) = -(far + near) / (far - near);
  out(2, 3) = -2 * far * near / (far - near);

  out(3, 2) = -1.f;
  return out;
}

Eigen::Matrix4f orthographicMatrix(const Float2 &size, float near, float far) {
  assert(size.x() < 90._radf);
  assert(size.y() < 90._radf);
  const Float2 xyScale = 2.0f * size.cwiseInverse();
  const float zScale = 2.0 / (near - far);
  const float m32 = near * zScale - 1.f;
  Eigen::Matrix4f out;
  // clang-format off
  out << xyScale.x(), 0.         , 0.     , 0.,
         0.         , xyScale.y(), 0.     , 0.,
         0.         , 0.         , zScale , 0.,
         0.         , 0.         , m32    , 1.;
  // clang-format on
  return out;
}
