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

} // namespace candlewick
