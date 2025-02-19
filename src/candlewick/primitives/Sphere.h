#pragma once

#include "../utils/MeshData.h"

namespace candlewick {

/// Load a sphere primitive organized in bottom-up rings and left-right
/// segments. The radius is assumed to be `1.0`.
MeshData loadUvSphereSolid(Uint32 rings, Uint32 segments);

} // namespace candlewick
