#include "Heightfield.h"

#include "Internal.h"

namespace candlewick {

MeshData loadHeightfield(const Eigen::Ref<const Eigen::MatrixXf> &heights,
                         const Eigen::Ref<const Eigen::VectorXf> &xgrid,
                         const Eigen::Ref<const Eigen::VectorXf> &ygrid) {
  SDL_assert(heights.rows() == xgrid.size());
  SDL_assert(heights.cols() == ygrid.size());
  const auto nx = (Sint32)heights.rows();
  const auto ny = (Sint32)heights.cols();
  SDL_assert(nx > 0);
  SDL_assert(ny > 0);
  Uint32 vertexCount = Uint32(nx * ny);
  std::vector<PosOnlyVertex> vertexData;
  std::vector<MeshData::IndexType> indexData;

  vertexData.reserve(vertexCount);
  indexData.resize(Uint32(4 * nx * (ny - 1)));
  size_t idx = 0;
  Sint32 ih, jh;
  // y-dir
  for (jh = 0; jh < ny; jh++) {
    // x-dir
    for (ih = 0; ih < nx; ih++) {
      Float3 pos{xgrid[ih], ygrid[jh], heights(ih, jh)};
      vertexData.emplace_back(pos);
      if (ih != nx - 1) {
        // connect x to x+dx
        indexData[idx++] = Uint32(jh * nx + ih);
        indexData[idx++] = Uint32(jh * nx + ih + 1);
      }
      if (jh != ny - 1) {
        // connect y to y+dy
        indexData[idx++] = Uint32(jh * nx + ih);
        indexData[idx++] = Uint32((jh + 1) * nx + ih);
      }
    }
  }
  assert(idx == indexData.size());

  return MeshData{SDL_GPU_PRIMITIVETYPE_LINELIST, std::move(vertexData),
                  std::move(indexData)};
}

} // namespace candlewick
