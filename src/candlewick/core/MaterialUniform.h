#pragma once

#include "math_types.h"

namespace candlewick {

struct alignas(16) PbrMaterialUniform {
  GpuVec4 baseColor;
  float metalness;
  float roughness;
  float ao;
};

} // namespace candlewick
