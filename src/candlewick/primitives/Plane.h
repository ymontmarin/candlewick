#pragma once

#include "../utils/MeshDataView.h"

namespace candlewick {

MeshDataView loadPlane();

MeshData loadPlaneTiled(float scale, Uint32 xrepeat, Uint32 yrepeat,
                        bool centered = true);

} // namespace candlewick
