#pragma once

#include "../utils/MeshData.h"

namespace candlewick {

/// Load a capsule primitive.
/// \note This will have a unit radius, for simplicity
MeshData loadCapsuleSolid(Uint32 hemisphereRings, Uint32 segments,
                          float length);

} // namespace candlewick
