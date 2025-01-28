#pragma once

#include "../core/math_types.h"
#include "../core/MaterialUniform.h"

#include "Utils.h"

#include <assimp/material.h>

namespace candlewick {

/// \brief PBR material for metallic-roughness workflow.
struct PbrMaterialData {
  float metalness = 0.f;
  float roughness = 1.0f;
  float ao = 1.0f;
  Float4 baseColor{1., 1., 1., 1.};

  PbrMaterialUniform toUniform() const {
    return {baseColor, metalness, roughness, ao};
  }
};

/// \brief Material parameters for a Blinn-Phong lighting model.
struct PhongMaterialData {
  Float4 diffuse;
  Float4 ambient{0.2f, 0.2f, 0.2f, 1.0f};
  Float4 specular{0.5f, 0.5f, 0.5f, 1.0f};
  Float4 emissive{Float4::Zero()};
  float shininess = 1.0f;
  float reflectivity = 0.0f;

  PhongMaterialUniform toUniform() const {
    return {diffuse, ambient, specular, shininess};
  }
};

/// \brief Load our PBR material data from an assimp material.
///
/// If the aiMaterial contains, in fact, a Phong material (\ref
/// PhongMaterialData), then a PBR material will be approximated.
PbrMaterialData loadFromAssimpMaterial(aiMaterial *material);

} // namespace candlewick
