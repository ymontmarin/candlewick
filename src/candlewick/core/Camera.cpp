#include "Camera.h"
#include "AABB.h"

namespace candlewick {

Mat4f lookAt(const Float3 &eye, const Float3 &center, const Float3 &up) {
  Mat4f mat;
  mat.setIdentity();
  auto R = mat.block<3, 3>(0, 0);
  Float3 zaxis = (eye - center).normalized();
  Float3 xaxis = up.cross(zaxis).normalized();
  Float3 yaxis = zaxis.cross(xaxis).normalized();
  R.row(0) = xaxis;
  R.row(1) = yaxis;
  R.row(2) = zaxis;
  // coords of eye in new reference frame
  auto tr = mat.col(3).head<3>();
  tr.noalias() = -R * eye;
  return mat;
}

Mat4f perspectiveFromFov(Radf fovY, float aspectRatio, float nearZ,
                         float farZ) {
  float f = 1.0f / std::tan(fovY * 0.5f);

  Mat4f result = Mat4f::Zero();
  result(0, 0) = f / aspectRatio;
  result(1, 1) = f;
  result(2, 2) = (farZ + nearZ) / (nearZ - farZ);
  result(3, 2) = -1.0f;
  result(2, 3) = (2.0f * farZ * nearZ) / (nearZ - farZ);
  return result;
}

Mat4f orthographicMatrix(float left, float right, float bottom, float top,
                         float near, float far) {
  const float sx = right - left;
  const float sy = top - bottom;
  const float zScale = 2.0f / (near - far);
  const float m23 = (near + far) / (near - far);
  const float px = (left + right) / sx;
  const float py = (top + bottom) / sy;
  Mat4f proj;
  // clang-format off
  proj << 2.f / sx, 0.      , 0.     , -px,
         0.       , 2.f / sy, 0.     , -py,
         0.       , 0.      , zScale , m23,
         0.       , 0.      , 0.     , 1.;
  // clang-format on
  return proj;
}

Mat4f orthographicMatrix(const Float2 &sizes, float near, float far) {
  const float sx = sizes.x();
  const float sy = sizes.y();
  const float zScale = 2.0f / (near - far);
  const float m23 = (near + far) / (near - far);
  Mat4f proj;
  // clang-format off
  proj << 2.f / sx, 0.      , 0.     , 0.,
         0.       , 2.f / sy, 0.     , 0.,
         0.       , 0.      , zScale , m23,
         0.       , 0.      , 0.     , 1.;
  // clang-format on
  return proj;
}
} // namespace candlewick
