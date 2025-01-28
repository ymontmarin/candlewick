#include <assimp/types.h>
#include "../core/math_types.h"

namespace candlewick {

inline Float4 assimpColor4ToEigen(aiColor4D color) {
  return Float4::Map(&color.r);
}

inline Float3 assimpColor3ToEigen(aiColor3D color) {
  return Float3::Map(&color.r);
}

} // namespace candlewick
