#pragma once

#include "../core/math_types.h"

namespace candlewick {

using FrustumCornersType = std::array<Float3, 8ul>;

inline FrustumCornersType frustumFromCameraProjection(const Mat4f &camProj) {
  auto invProj = camProj.inverse();
  FrustumCornersType out;
  for (uint8_t i = 0; i < 8; i++) {
    uint8_t l = i & 1;
    uint8_t k = (i >> 1) & 1;
    uint8_t j = (i >> 2) & 1;

    Float4 ndc{2.f * l - 1.f, 2.f * j - 1.f, 2.f * k - 1.f, 1.f};
    Float4 viewSpace = invProj * ndc;
    out[i] = viewSpace.head<3>() / viewSpace.w();
  }
  return out;
}

} // namespace candlewick
