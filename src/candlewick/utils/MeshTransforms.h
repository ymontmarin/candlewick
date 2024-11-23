#pragma once

#include "Utils.h"
#include <Eigen/Geometry>

namespace candlewick {

void applyTransformToMeshInPlace(MeshData &meshData, const Eigen::Affine3f &tr);

}
