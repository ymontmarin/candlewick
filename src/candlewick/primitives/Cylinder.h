#pragma once

#include "../utils/MeshData.h"

namespace candlewick {

/// \brief Load a solid 3D cylinder.
/// \ingroup primitives1
MeshData loadCylinderSolid(Uint32 rings, Uint32 segments, float radius,
                           float height);

} // namespace candlewick
