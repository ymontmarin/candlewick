#pragma once

#include "math_types.h"

namespace candlewick {

struct alignas(16) DirectionalLightUniform {
  GpuVec3 direction;
  alignas(16) GpuVec3 color;
  float intensity;
};
static_assert(std::is_standard_layout_v<DirectionalLightUniform>);
static_assert(offsetof(DirectionalLightUniform, color) == 16);

} // namespace candlewick
