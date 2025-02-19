#pragma once

#include "../utils/MeshData.h"

namespace candlewick {

/// \brief Load a sphere primitive organized in bottom-up rings and left-right
/// segments.
///
/// This will have unit radius. You can apply a scaling transform to change the
/// radius to what you want. See apply3DTransformInPlace().
/// \ingroup primitives1
MeshData loadUvSphereSolid(Uint32 rings, Uint32 segments);

} // namespace candlewick
