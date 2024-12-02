#include "Grid.h"
#include <SDL3/SDL_assert.h>

namespace candlewick {

struct alignas(16) MyVertex {
  GpuVec3 pos;
};

template <> struct VertexTraits<MyVertex> {
  constexpr static auto layout() {
    return MeshLayout{}
        .addBinding(0, sizeof(MyVertex))
        .addAttribute("pos", 0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                      offsetof(MyVertex, pos));
  }
};

MeshData loadGrid(Uint32 xyHalfSize) {
  // one square = 50cm
  constexpr float scale = 0.5f;
  const Uint32 size = std::max(2 * xyHalfSize, 1u) - 1;
  std::vector<MyVertex> vertexData;
  std::vector<MeshData::IndexType> indexData;

  vertexData.reserve(size * size);
  indexData.resize(4 * size * (size - 1));
  const Float3 center{float(xyHalfSize) * scale, float(xyHalfSize) * scale,
                      0.f};

  size_t idx = 0;
  Uint32 i, j;
  // y-direction
  for (j = 0; j < size; j++) {
    // x-direction
    for (i = 0; i < size; i++) {
      Float3 pos{float(i) * scale, float(j) * scale, 0.f};
      pos -= center;
      vertexData.emplace_back(pos);
      if (i != size - 1) {
        // (i,j) -- (i+1,j)
        indexData[idx++] = Uint32(j * size + i);
        indexData[idx++] = Uint32(j * size + i + 1);
      }
      if (j != size - 1) {
        // (i,j)
        //   |
        // (i,j+1)
        indexData[idx++] = Uint32(j * size + i);
        indexData[idx++] = Uint32((j + 1) * size + i);
      }
    }
  }
  return MeshData{SDL_GPU_PRIMITIVETYPE_LINELIST, std::move(vertexData),
                  std::move(indexData)};
}

} // namespace candlewick
