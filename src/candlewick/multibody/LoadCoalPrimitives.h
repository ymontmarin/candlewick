#pragma once

#include "../utils/Utils.h"
#include "../core/math_types.h"

#include <hpp/fcl/fwd.hh>

namespace candlewick {

MeshData loadCoalPrimitive(const hpp::fcl::CollisionGeometry &geometry,
                           const Float4 &meshColor, const Float3 &meshScale);

}
