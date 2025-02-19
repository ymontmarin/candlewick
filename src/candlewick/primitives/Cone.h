#pragma once

#include "../utils/MeshData.h"

namespace candlewick {

/// \brief Load a 3D solid cone.
/// \ingroup primitives1
MeshData loadConeSolid(Uint32 segments, float radius, float length);

} // namespace candlewick
