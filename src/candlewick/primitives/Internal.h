#pragma once
#include "../core/math_types.h"
#include "../core/MeshLayout.h"

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

} // namespace candlewick
