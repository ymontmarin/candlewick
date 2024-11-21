#pragma once

#include "math_types.h"

namespace candlewick {

inline double deg2rad(double t) { return t * M_PI / 180.0; }
inline float deg2rad(float t) { return t * M_PIf / 180.0f; }
inline float rad2deg(float t) { return t * 180.0f / M_PIf; }

inline Eigen::Vector3i hexToRgbi(unsigned long hex) {
  // red = quotient of hex by 256^2
  int r = hex / (256 * 256);
  // remainder
  hex -= r;
  // green = long division of remainder by 256
  int g = hex / 256;
  hex -= g;
  // blue = remainder
  int b = hex;
  Eigen::Vector3i result{r, g, b};
  assert(result.x() < 256);
  assert(result.y() < 256);
  assert(result.z() < 256);
  return result;
};

inline Eigen::Vector4i hexToRgbai(unsigned long hex) {
  unsigned rem = hex;
  // red = quotient of hex by 256^2
  int r = rem >> 24;
  // remainder
  rem %= 1 << 24;
  // green = long division of remainder by 256
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

inline Float3 hexToRgbf(unsigned long hex) {
  Eigen::Vector3i res = hexToRgbi(hex);
  return res.cast<float>() / 255.;
};

inline Float4 hexToRgbaf(unsigned long hex) {
  Eigen::Vector4i res = hexToRgbai(hex);
  Float4 out = res.cast<float>() / 255.;
  printf("Convert hex %zx to color rgba(%f,%f,%f,%f)\n", hex, out.x(), out.y(),
         out.z(), out.w());
  return out;
};

inline Rad<float> operator""_radf(long double t) {
  return Rad<float>(deg2rad(static_cast<float>(t)));
}

inline Rad<double> operator""_rad(long double t) {
  return Rad<double>(deg2rad(static_cast<double>(t)));
}

inline Float3 operator""_rgbf(unsigned long long hex) { return hexToRgbf(hex); }

inline Float4 operator""_rgbaf(unsigned long long hex) {
  return hexToRgbaf(hex);
}

/// Compute view matrix looking at \p center from \p eye, with
/// the camera pointing up towards \p up.
Eigen::Matrix4f lookAt(const Float3 &eye, const Float3 &center,
                       const Float3 &up = Float3::UnitZ());

/// Compute orthographic projection matrix, from clipping plane parameters
/// (left, right, top, bottom).
Eigen::Matrix4f perspectiveMatrix(float left, float right, float top,
                                  float bottom, float near, float far);

Eigen::Matrix4f
perspectiveFromFov(float fovY,        // Vertical field of view in radians
                   float aspectRatio, // Width / Height
                   float nearZ,       // Near clipping plane
                   float farZ         // Far clipping plane
);

Eigen::Matrix4f orthographicMatrix(const Float2 &size, float, float far);

inline void orthographicZoom(Eigen::Matrix4f &projMatrix, float factor) {
  projMatrix(0, 0) *= 1. / factor;
  projMatrix(1, 1) *= 1. / factor;
}

} // namespace candlewick
