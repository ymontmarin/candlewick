#pragma once

#include "../utils/MeshData.h"

namespace candlewick {

/// \brief Load a capsule primitive.
///
/// This will have unit radius. You can apply a scaling transform to change the
/// radius to what you want. See apply3DTransformInPlace().
/// \ingroup primitives1
MeshData loadCapsuleSolid(Uint32 hemisphereRings, Uint32 segments,
                          float length);

} // namespace candlewick
