#pragma once

#include "../utils/MeshDataView.h"

namespace candlewick {

/// \ingroup primitives1
MeshDataView loadPlane();

/// \ingroup primitives1
MeshData loadPlaneTiled(float scale, Uint32 xrepeat, Uint32 yrepeat,
                        bool centered = true);

} // namespace candlewick
