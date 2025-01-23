#pragma once

#include "../utils/Frustum.h"
#include "../utils/MeshData.h"

namespace candlewick {

/// \brief Load a MeshData for a camera frustum, given its corners.
/// \warning The corners are expected to be created in lexicographical order!
/// \sa frustumFromCameraProjection()
MeshData loadFrustumFromCorners(const FrustumCornersType &corners);

} // namespace candlewick
