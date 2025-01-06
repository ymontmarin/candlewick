#pragma once

#include "candlewick/core/Mesh.h"
#include "candlewick/utils/MaterialData.h"

namespace candlewick {

/// \brief Simple struct holding draw data for a set of meshes with materials.
struct ShapeData {
  std::vector<Mesh> meshes;
  std::vector<PbrMaterialData> materials;
  bool hasMaterials() const { return !materials.empty(); }
};
} // namespace candlewick
