#pragma once

#include "Multibody.h"
#include "../utils/MeshData.h"

#include <pinocchio/multibody/geometry-object.hpp>

namespace candlewick::multibody {

/// \brief Load an invidual Pinocchio GeometryObject's component geometries into
/// an array of \c MeshData.
void loadGeometryObject(const pin::GeometryObject &gobj,
                        std::vector<MeshData> &meshData);

inline std::vector<MeshData>
loadGeometryObject(const pin::GeometryObject &gobj) {
  std::vector<MeshData> meshData;
  loadGeometryObject(gobj, meshData);
  return meshData;
}

} // namespace candlewick::multibody
