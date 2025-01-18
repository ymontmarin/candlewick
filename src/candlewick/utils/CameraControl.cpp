#include "CameraControl.h"

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

Mat4f perspectiveMatrix(float left, float right, float bottom, float top,
                        float near, float far) {
  float hfov = right - left;
  float vfov = top - bottom;
  float aspect = hfov / vfov;
  return perspectiveFromFov(vfov, aspect, near, far);
}

Mat4f orthographicMatrix(float left, float right, float bottom, float top,
                         float near, float far) {
  Mat4f proj;
  proj.setZero();
  proj(0, 0) = 2.0f / (right - left);
  proj(1, 1) = 2.0f / (top - bottom);
  proj(2, 2) = -2.0f / (far - near);

  proj(0, 3) = -(right + left) / (right - left);
  proj(1, 3) = -(top + bottom) / (top - bottom);
  proj(2, 3) = -(far + near) / (far - near);
  proj(3, 3) = 1.0f;

  return proj;
}

Mat4f orthographicMatrix(const Float2 &size, float near, float far) {
  assert(size.y() < 90._radf);
  const Float2 xyScale = 2.0f * size.cwiseInverse();
  const float zScale = 2.0f / (far - near);
  const float m23 = -(far + near) / (far - near);
  Mat4f out;
  // clang-format off
  out << xyScale.x(), 0.         , 0.     , 0.,
         0.         , xyScale.y(), 0.     , 0.,
         0.         , 0.         , -zScale, m23,
         0.         , 0.         , 0.     , 1.;
  // clang-format on
  return out;
}

} // namespace candlewick
