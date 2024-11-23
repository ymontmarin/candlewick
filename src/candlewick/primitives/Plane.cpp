#include "Plane.h"

namespace candlewick {

MeshData::Vertex vertexData[]{{{1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                              {{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                              {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                              {{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}};

MeshDataView loadPlane() {
  return {SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP, vertexData};
}

} // namespace candlewick
