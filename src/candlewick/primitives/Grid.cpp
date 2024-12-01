#include "../core/matrix_util.h"
#include "../utils/MeshDataView.h"
#include "../utils/MeshTransforms.h"

#include <SDL3/SDL_assert.h>

namespace candlewick {

const Float4 color = 0xE0A236ff_rgbaf;

// 1—3
// | |
// 0—2
const DefaultVertex quad[] = {
    {{0.f, 0.f, 0.f}, Float3{}, color},
    {{0.f, 1.f, 0.f}, Float3{}, color},
    {{1.f, 0.f, 0.f}, Float3{}, color},
    {{1.f, 1.f, 0.f}, Float3{}, color},
};

constexpr Uint32 indexData[] = {
    0, 1, //
    0, 2, //
    1, 3, //
    2, 3, //
};

MeshDataView loadGridElement() {
  return {SDL_GPU_PRIMITIVETYPE_LINELIST, quad, indexData};
};

MeshData loadGrid(Uint32 xyHalfSize) {
  // one square = 50cm
  constexpr float scale = 0.5f;
  MeshData dataOrig = toOwningMeshData(loadGridElement());
  std::vector<MeshData> meshDatas;
  const Uint32 size = std::max(2 * xyHalfSize, 1u) - 1;
  meshDatas.reserve(size * size);

  Sint32 i, j;
  for (i = -Sint32(xyHalfSize) + 1; i < Sint32(xyHalfSize); i++) {
    for (j = -Sint32(xyHalfSize) + 1; j < Sint32(xyHalfSize); j++) {
      MeshData &m = meshDatas.emplace_back(toOwningMeshData(dataOrig));
      Eigen::Affine3f tr;
      tr.setIdentity();
      tr.scale(scale);
      tr.translate(Float3{float(i), float(j), 0.f});
      apply3DTransformInPlace(m, tr);
    }
  }
  SDL_assert(i == size);
  SDL_assert(j == size);
  return mergeMeshes(meshDatas);
}

} // namespace candlewick
