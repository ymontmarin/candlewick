#pragma once

#include "math_types.h"

namespace candlewick {

inline double deg2rad(double t) { return t * M_PI / 180.0; }
inline float deg2rad(float t) { return t * M_PIf / 180.0f; }
inline float rad2deg(float t) { return t * 180.0f / M_PIf; }

inline Rad<float> operator""_radf(long double t) {
  return Rad<float>(deg2rad(static_cast<float>(t)));
}

inline Rad<double> operator""_rad(long double t) {
  return Rad<double>(deg2rad(static_cast<double>(t)));
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
