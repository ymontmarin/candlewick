#pragma once

#include "../utils/MeshData.h"

namespace candlewick {

/// \brief Load a heightfield, as line geometry.
/// \ingroup primitives1
MeshData loadHeightfield(const Eigen::Ref<const Eigen::MatrixXf> &heights,
                         const Eigen::Ref<const Eigen::VectorXf> &xgrid,
                         const Eigen::Ref<const Eigen::VectorXf> &ygrid);
} // namespace candlewick
