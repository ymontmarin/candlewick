#pragma once

#include "../utils/MeshData.h"

namespace candlewick {
MeshData loadCylinder(Uint32 rings, Uint32 segments, float radius,
                      float height);
}
