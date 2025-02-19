#pragma once

#include "../utils/MeshData.h"

namespace candlewick {
MeshData loadCylinderSolid(Uint32 rings, Uint32 segments, float radius,
                           float height);
}
