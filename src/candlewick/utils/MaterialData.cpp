#include "MaterialData.h"

#include "../third-party/magic_enum.hpp"
#include "assimp_convert.h"

namespace candlewick {

std::string_view assimpPropertyTypeName(aiPropertyTypeInfo type) {
  return magic_enum::enum_name(type);
}

#define _col(name) (aiColor4D &)*name.data()

using namespace magic_enum::bitwise_operators;

PhongMaterialData loadPhongMaterial(aiMaterial *material) {
  PhongMaterialData res;
  aiReturn flag;
  flag &= material->Get(AI_MATKEY_COLOR_DIFFUSE, _col(res.diffuse));
  material->Get(AI_MATKEY_COLOR_AMBIENT, _col(res.ambient));
  material->Get(AI_MATKEY_COLOR_SPECULAR, _col(res.specular));
  material->Get(AI_MATKEY_COLOR_EMISSIVE, _col(res.emissive));
  flag &= material->Get(AI_MATKEY_SHININESS, res.shininess);
  material->Get(AI_MATKEY_REFLECTIVITY, res.reflectivity);

  return res;
}

PbrMaterialData pbrFromPhong(const PhongMaterialData &phong) {
  PbrMaterialData out;
  out.baseColor = phong.diffuse;
  float specAvg = phong.specular.head<3>().mean();
  float specVar =
      (phong.specular.head<3>() - Float3::Constant(specAvg)).squaredNorm();

  const float METALLIC_THRESHOLD = 0.2f;
  if (specVar > METALLIC_THRESHOLD) {
    out.metalness = .5f;
    out.baseColor = phong.specular;
  } else {
    out.metalness = 0.f;
    out.baseColor *= (1.0f + specAvg);
  }

  float shininess = std::clamp(phong.shininess, 1.0f, 1e3f);
  out.roughness = sqrtf(2.f / (shininess + 2.f));
  return out;
}

PbrMaterialData loadFromAssimpMaterial(aiMaterial *material) {
  PbrMaterialData out;
  aiReturn flag;
  flag &= material->Get(AI_MATKEY_BASE_COLOR, _col(out.baseColor));
  flag &= material->Get(AI_MATKEY_METALLIC_FACTOR, out.metalness);
  material->Get(AI_MATKEY_ROUGHNESS_FACTOR, out.roughness);
  out.ao = 1.0f;

  PhongMaterialData phong = loadPhongMaterial(material);
  out = pbrFromPhong(phong);
  return out;
}

#undef _col
} // namespace candlewick
