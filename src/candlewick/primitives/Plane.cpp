#include "Plane.h"
#include "../utils/MeshTransforms.h"

namespace candlewick {

// 3——1
// │ /│
// │/ │
// 2——0
MeshData::Vertex vertexData[]{{{1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                              {{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                              {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                              {{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}};

MeshDataView loadPlane() {
  return {SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP, vertexData};
}

MeshData loadPlaneTiled(float scale, Uint32 xrepeat, Uint32 yrepeat,
                        bool centered) {
  MeshData dataOwned = generateIndices(toOwningMeshData(loadPlane()));
  {
    // normalize to (-1,-1) -- (1,1)
    const Eigen::Translation3f tr{0.5, 0.5};
    apply3DTransformInPlace(dataOwned, Eigen::Affine3f(tr).scale(0.5));
  }
  std::vector<MeshData> meshes;
  meshes.reserve(xrepeat * yrepeat);
  for (Sint32 i = 0; i < Sint32(xrepeat); i++) {
    for (Sint32 j = 0; j < Sint32(yrepeat); j++) {
      MeshData &m = meshes.emplace_back(toOwningMeshData(dataOwned));
      const Eigen::Translation3f tr{i * scale, j * scale, 0.};
      apply3DTransformInPlace(m, Eigen::Affine3f(tr).scale(scale));
    }
  }
  MeshData out = mergeMeshes(meshes);
  if (centered) {
    float xc = 0.5f * scale * xrepeat;
    float yc = 0.5f * scale * yrepeat;
    const Eigen::Translation3f center{-xc, -yc, 0.f};
    apply3DTransformInPlace(out, Eigen::Affine3f{center});
  }
  return out;
}

} // namespace candlewick
