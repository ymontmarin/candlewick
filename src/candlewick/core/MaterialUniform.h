#pragma once

#include "math_types.h"

namespace candlewick {

struct alignas(16) PbrMaterialUniform {
  float metalness;
  float roughness;
  float ao;
  GpuVec4 baseColor;
};

} // namespace candlewick
