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

inline Eigen::Vector3d operator""_rgb(unsigned long long hex) {
  return hexToRgbi(hex).cast<double>() / 255.;
}

inline Eigen::Vector4d operator""_rgba(unsigned long long hex) {
  return hexToRgbai(hex).cast<double>() / 255.;
}

} // namespace candlewick
