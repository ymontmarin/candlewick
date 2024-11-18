#pragma once

#include "../core/math_types.h"
#include "../core/MaterialUniform.h"
#include <assimp/material.h>

namespace candlewick {

struct PbrMaterialData {
  float metalness = 0.f;
  float roughness = 1.0f;
  float ao = 1.0f;
  Float4 baseColor;
  std::vector<std::string> texturePaths;

  PbrMaterialUniform toUniform() const {
    return {metalness, roughness, ao, baseColor};
  }
};

PbrMaterialData loadFromAssimpMaterial(aiMaterial *material);

} // namespace candlewick
