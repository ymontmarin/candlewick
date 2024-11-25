#include "math_util.h"

#include <Eigen/Geometry>

namespace candlewick {

Eigen::Vector3i hexToRgbi(unsigned long hex) {
  unsigned rem = hex;
  // red = quotient of hex by 256^2 = 2^16
  int r = rem >> 16;
  // remainder
  rem %= 1 << 16;
  // green = long division of remainder by 256
  int g = rem >> 8;
  rem %= 1 << 8;
  // blue = remainder
  int b = rem;
  Eigen::Vector3i result{r, g, b};
  assert(result.x() < 256);
  assert(result.y() < 256);
  assert(result.z() < 256);
  return result;
};

Eigen::Vector4i hexToRgbai(unsigned long hex) {
  unsigned rem = hex;
  // red = quotient of hex by 256^3 = 2^24
  int r = rem >> 24;
  // remainder
  rem %= 1 << 24;
  // green = long division of remainder by 256^2 = 2^16
  int g = rem >> 16;
  rem %= 1 << 16;
  // blue = quotient of remainder
  int b = rem >> 8;
  rem %= 1 << 8;
  // alpha = remainder
  int a = rem;
  Eigen::Vector4i result{r, g, b, a};
  assert(result.x() < 256);
  assert(result.y() < 256);
  assert(result.z() < 256);
  assert(result.w() < 256);
  printf("Convert hex %zx to color rgba(%d,%d,%d,%d)\n", hex, r, g, b, a);
  return result;
};

Eigen::Matrix4f lookAt(const Float3 &eye, const Float3 &center,
                       const Float3 &up) {
  Eigen::Matrix4f mat;
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

Eigen::Matrix4f perspectiveFromFov(float fovY, float aspectRatio, float nearZ,
                                   float farZ) {
  float f = 1.0f / std::tan(fovY * 0.5f);

  Eigen::Matrix4f result = Eigen::Matrix4f::Zero();
  result(0, 0) = f / aspectRatio;
  result(1, 1) = f;
  result(2, 2) = (farZ + nearZ) / (nearZ - farZ);
  result(3, 2) = -1.0f;
  // translation component along Z
  result(2, 3) = (2.0f * farZ * nearZ) / (nearZ - farZ);

  return result;
}

Eigen::Matrix4f perspectiveMatrix(float left, float right, float top,
                                  float bottom, float near, float far) {
  float hfov = right - left;
  float vfov = top - bottom;
  float aspect = hfov / vfov;
  return perspectiveFromFov(vfov, aspect, near, far);
}

Eigen::Matrix4f orthographicMatrix(const Float2 &size, float near, float far) {
  assert(size.y() < 90._radf);
  const Float2 xyScale = 2.0f * size.cwiseInverse();
  const float zScale = 2.0f / (near - far);
  const float m23 = near * zScale - 1.f;
  Eigen::Matrix4f out;
  // clang-format off
  out << xyScale.x(), 0.         , 0.     , 0.,
         0.         , xyScale.y(), 0.     , 0.,
         0.         , 0.         , zScale , m23,
         0.         , 0.         , 0.     , 1.;
  // clang-format on
  return out;
}
} // namespace candlewick
