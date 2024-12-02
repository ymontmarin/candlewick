#pragma once

#include "../core/math_types.h"
#include "../core/MeshLayout.h"

namespace candlewick {

struct alignas(16) DefaultVertex {
  GpuVec3 pos;
  alignas(16) GpuVec3 normal;
  alignas(16) GpuVec4 color;
};
static_assert(IsVertexType<DefaultVertex>, "");

template <> struct VertexTraits<DefaultVertex> {
  static constexpr auto layout() {
    return MeshLayout{}
        .addBinding(0, sizeof(DefaultVertex))
        .addAttribute("pos", 0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                      offsetof(DefaultVertex, pos))
        .addAttribute("normal", 1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                      offsetof(DefaultVertex, normal))
        .addAttribute("color", 2, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
                      offsetof(DefaultVertex, color));
  }
};

} // namespace candlewick
