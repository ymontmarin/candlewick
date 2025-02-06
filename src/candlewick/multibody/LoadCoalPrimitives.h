#pragma once

#include "../utils/Utils.h"
#include "../core/math_types.h"

#include <coal/fwd.hh>

namespace candlewick {

MeshData loadCoalPrimitive(const coal::CollisionGeometry &geometry,
                           const Float4 &meshColor, const Float3 &meshScale);

MeshData loadCoalHeightField(const coal::CollisionGeometry &collGeom);

} // namespace candlewick
