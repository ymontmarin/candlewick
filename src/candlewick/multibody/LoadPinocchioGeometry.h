#pragma once

#include "Multibody.h"
#include "../utils/MeshData.h"

#include <pinocchio/multibody/geometry-object.hpp>

namespace candlewick::multibody {

void loadGeometryObject(const pin::GeometryObject &gobj,
                        std::vector<MeshData> &meshData);

void loadGeometryModel(const pin::GeometryModel &model,
                       std::vector<MeshData> &meshData);

} // namespace candlewick::multibody
