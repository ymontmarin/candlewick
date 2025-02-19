#pragma once

#include "../utils/MeshData.h"

namespace candlewick {

/// \brief Load a line grid.
/// \ingroup primitives1
MeshData loadGrid(Uint32 xyHalfSize, float scale = 0.5f);

} // namespace candlewick
