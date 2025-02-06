#pragma once

#include "../utils/MeshData.h"

namespace candlewick {

/// \brief Load a heightfield, as line geometry.
MeshData loadHeightfield(const Eigen::Ref<const Eigen::MatrixXf> &heights,
                         const Eigen::Ref<const Eigen::VectorXf> &xgrid,
                         const Eigen::Ref<const Eigen::VectorXf> &ygrid);
} // namespace candlewick
