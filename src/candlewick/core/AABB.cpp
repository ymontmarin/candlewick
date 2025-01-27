#include "AABB.h"
#include "Camera.h"

namespace candlewick {

Mat4f orthoFromAABB(const AABB &sceneBounds) {
  return orthographicMatrix(sceneBounds.min.x(), sceneBounds.max.x(),
                            sceneBounds.min.y(), sceneBounds.max.y(),
                            -sceneBounds.max.z(), -sceneBounds.min.z());
}

} // namespace candlewick
