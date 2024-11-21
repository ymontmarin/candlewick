#pragma once

#include "math_types.h"

namespace candlewick {

struct alignas(16) DirectionalLightUniform {
  GpuVec3 direction;
  alignas(16) GpuVec3 color;
  float intensity;
};

} // namespace candlewick
