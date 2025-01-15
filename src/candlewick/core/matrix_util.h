#pragma once

#include "math_types.h"

namespace candlewick {

Vec3u8 hexToRgbi(unsigned long hex);

Vec4u8 hexToRgbai(unsigned long hex);

inline Float3 hexToRgbf(unsigned long hex) {
  return hexToRgbi(hex).cast<float>() / 255.f;
};

inline Float4 hexToRgbaf(unsigned long hex) {
  return hexToRgbai(hex).cast<float>() / 255.f;
};

inline Float3 operator""_rgbf(unsigned long long hex) { return hexToRgbf(hex); }

inline Float4 operator""_rgbaf(unsigned long long hex) {
  return hexToRgbaf(hex);
}

/// Compute view matrix looking at \p center from \p eye, with
/// the camera pointing up towards \p up.
Eigen::Matrix4f lookAt(const Float3 &eye, const Float3 &center,
                       const Float3 &up = Float3::UnitZ());

/// \brief Compute orthographic projection matrix, from clipping plane
/// parameters (left, right, top, bottom).
Eigen::Matrix4f perspectiveMatrix(float left, float right, float top,
                                  float bottom, float near, float far);

/// \brief Get perspective projection matrix given fov, aspect ratio, and
/// clipping planes.
/// \p fovY Vertical field of view in radians
/// \p aspectRatio Width / Height
/// \p nearZ Near clipping plane
/// \p farZ Far clipping plane
Eigen::Matrix4f perspectiveFromFov(Radf fovY, float aspectRatio, float nearZ,
                                   float farZ);

Eigen::Matrix4f orthographicMatrix(const Float2 &size, float, float far);

inline void orthographicZoom(Eigen::Matrix4f &projMatrix, float factor) {
  projMatrix(0, 0) *= 1.f / factor;
  projMatrix(1, 1) *= 1.f / factor;
}

} // namespace candlewick
