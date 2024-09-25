#pragma once

#include <SDL3/SDL_gpu.h>
#include <vector>

namespace candlewick {

/// Struct which defines the layout of a mesh's vertices.
/// This is a clever wrapper around
struct MeshLayout {
  explicit MeshLayout() = default;

  /// @p slot - index for vertex buffer
  /// @p size - equivalent of pitch in SDL_gpu, size of consecutive elements of
  /// the vertex buffer, i.e. sizeof(Vertex) if your vertices are of some type
  /// Vertex.
  MeshLayout &addBinding(Uint32 slot, Uint32 pitch) &;
  MeshLayout &&addBinding(Uint32 slot, Uint32 pitch) &&;

  MeshLayout &addAttribute(Uint32 loc, Uint32 binding,
                           SDL_GPUVertexElementFormat format, Uint32 offset) &;
  MeshLayout &&addAttribute(Uint32 loc, Uint32 binding,
                            SDL_GPUVertexElementFormat format,
                            Uint32 offset) &&;

  SDL_GPUVertexInputState toVertexInputState();

private:
  std::vector<SDL_GPUVertexBufferDescription> vertex_buffer_desc;
  std::vector<SDL_GPUVertexAttribute> vertex_attributes;
};

} // namespace candlewick
