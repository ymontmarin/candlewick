#include "MeshLayout.h"
#include <SDL3/SDL_gpu.h>

namespace candlewick {

MeshLayout &MeshLayout::addBinding(Uint32 slot, Uint32 pitch) & {
  vertex_buffer_desc.push_back({
      .slot = slot,
      .pitch = pitch,
      .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
      .instance_step_rate = 0,
  });
  return *this;
}

MeshLayout &&MeshLayout::addBinding(Uint32 slot, Uint32 pitch) && {
  return std::move(addBinding(slot, pitch));
}

MeshLayout &MeshLayout::addAttribute(Uint32 loc, Uint32 binding,
                                     SDL_GPUVertexElementFormat format,
                                     Uint32 offset) & {
  vertex_attributes.push_back({loc, binding, format, offset});
  return *this;
}

MeshLayout &&MeshLayout::addAttribute(Uint32 loc, Uint32 binding,
                                      SDL_GPUVertexElementFormat format,
                                      Uint32 offset) && {
  return std::move(addAttribute(loc, binding, format, offset));
}

SDL_GPUVertexInputState MeshLayout::toVertexInputState() {
  return {vertex_buffer_desc.data(), (Uint32)vertex_buffer_desc.size(),
          vertex_attributes.data(), (Uint32)vertex_attributes.size()};
}

} // namespace candlewick
