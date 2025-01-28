#pragma once

#include "math_types.h"

namespace candlewick {

struct alignas(16) PbrMaterialUniform {
  GpuVec4 baseColor;
  float metalness;
  float roughness;
  float ao;
};

struct alignas(16) PhongMaterialUniform {
  GpuVec4 diffuse;
  GpuVec4 ambient;
  GpuVec4 specular;
  float shininess;
};

} // namespace candlewick
