#pragma once

#include "math_types.h"

namespace candlewick {

struct alignas(16) DirectionalLight {
  GpuVec3 direction;
  alignas(16) GpuVec3 color;
  float intensity;
};
static_assert(std::is_standard_layout_v<DirectionalLight>);

} // namespace candlewick
