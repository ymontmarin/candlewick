#include "MaterialData.h"
#include "assimp_convert.h"

namespace candlewick {

PbrMaterialData loadFromAssimpMaterial(aiMaterial *material) {
  PbrMaterialData out;
  aiColor4D baseColor;
  material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor);
  out.baseColor = util::assimpColor4ToEigen(baseColor);

  material->Get(AI_MATKEY_METALLIC_FACTOR, out.metalness);
  material->Get(AI_MATKEY_ROUGHNESS_FACTOR, out.roughness);
  out.ao = 1.0f;
  return out;
}

} // namespace candlewick
