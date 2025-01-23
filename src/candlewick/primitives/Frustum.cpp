#include "Frustum.h"
#include "Internal.h"

#include "../utils/MeshDataView.h"

namespace candlewick {

constexpr Uint32 indexData[]{
    0, 1, // x-edge
    0, 2, // y-edge
    2, 3, // x-edge
    1, 3, // y-edge

    // Top face (z=1)
    4, 5, // x-edge
    4, 6, // y-edge
    6, 7, // x-edge
    5, 7, // y-edge

    // Vertical edges
    0, 4, // z-edge
    1, 5, // z-edge
    2, 6, // z-edge
    3, 7  // z-edge
};

MeshData loadFrustumFromCorners(const FrustumCornersType &corners) {
  PosOnlyVertex vertexData[8];
  for (Uint8 i = 0; i < 8; i++) {
    vertexData[i] = {corners[i]};
  }

  // there are 12 edges.
  MeshDataView dataView{SDL_GPU_PRIMITIVETYPE_LINELIST, vertexData, indexData};
  return toOwningMeshData(dataView);
}

} // namespace candlewick
