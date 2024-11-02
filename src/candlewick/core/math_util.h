#pragma once

#include "math_types.h"

namespace candlewick {

inline double deg2rad(double t) { return t * M_PI / 180.0; }
inline float deg2rad(float t) { return t * M_PIf / 180.0f; }
inline float operator""_radf(long double t) {
  return deg2rad(static_cast<float>(t));
}
inline double operator""_rad(long double t) {
  return deg2rad(static_cast<double>(t));
}

/// Compute view matrix looking at \p center from \p eye, with
/// the camera pointing up towards \p up.
Eigen::Matrix4f lookAt(const Float3 &eye, const Float3 &center,
                       const Float3 &up);

/// Compute perspective matrix, from clipping plane parameters
/// (left, right, top, bottom).
Eigen::Matrix4f perspectiveMatrix(float left, float right, float top,
                                  float bottom, float near, float far);

Eigen::Matrix4f orthographicMatrix(const Float2 &size, float, float far);

} // namespace candlewick
