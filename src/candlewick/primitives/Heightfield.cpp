#include "Heightfield.h"

namespace candlewick {

struct alignas(16) PosOnlyVertex {
  GpuVec3 pos;
};

template <> struct VertexTraits<PosOnlyVertex> {
  static auto layout() {
    return MeshLayout{}
        .addBinding(0, sizeof(PosOnlyVertex))
        .addAttribute("pos", 0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                      offsetof(PosOnlyVertex, pos));
  }
};

MeshData loadHeightfield(const Eigen::MatrixXf &heights,
                         const Eigen::VectorXf &xgrid,
                         const Eigen::VectorXf &ygrid) {
  SDL_assert(heights.rows() == xgrid.size());
  SDL_assert(heights.cols() == ygrid.size());
  auto nx = (Sint32)heights.rows();
  auto ny = (Sint32)heights.cols();
  SDL_assert(nx > 0);
  SDL_assert(ny > 0);
  Uint32 vertexCount = Uint32(nx * ny);
  std::vector<PosOnlyVertex> vertexData{vertexCount};
  std::vector<MeshData::IndexType> indexData;
  indexData.resize(Uint32(4 * nx * ny));
  size_t idx = 0;
  Sint32 ih, jh;
  // y-dir
  for (jh = 0; jh < ny; jh++) {
    // x-dir
    for (ih = 0; ih < nx; ih++) {
      vertexData.push_back({.pos{xgrid[ih], ygrid[jh], heights(ih, jh)}});
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

  return MeshData{SDL_GPU_PRIMITIVETYPE_LINELIST, std::move(vertexData),
                  std::move(indexData)};
}

} // namespace candlewick
