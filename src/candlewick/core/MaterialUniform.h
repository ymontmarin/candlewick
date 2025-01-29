#pragma once

#include "math_types.h"

namespace candlewick {

/// \brief PBR material for metallic-roughness workflow.
struct alignas(16) PbrMaterial {
  GpuVec4 baseColor{1., 1., 1., 1.};
  float metalness = 0.f;
  float roughness = 1.0f;
  float ao = 1.0f;
};

/// \brief Material parameters for a Blinn-Phong lighting model.
struct alignas(16) PhongMaterial {
  GpuVec4 diffuse;
  GpuVec4 ambient{0.2f, 0.2f, 0.2f, 1.0f};
  GpuVec4 specular{0.5f, 0.5f, 0.5f, 1.0f};
  GpuVec4 emissive{Float4::Zero()};
  float shininess = 1.0f;
  float reflectivity = 0.0f;
};

} // namespace candlewick
