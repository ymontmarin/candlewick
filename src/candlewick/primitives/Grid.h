#pragma once

#include "../utils/MeshDataView.h"

namespace candlewick {

MeshDataView loadGridElement();

MeshData loadGrid(Uint32 xyHalfSize);

} // namespace candlewick
