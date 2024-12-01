#pragma once

#include <SDL3/SDL_gpu.h>
#include <utility>

namespace candlewick {

static constexpr Uint32 MAX_VERTEX_BUF_DESCS = 10u;
static constexpr Uint32 MAX_VERTEX_ATTRS = 10u;

/// Struct which defines the layout of a mesh's vertices.
/// This is a clever wrapper around
struct MeshLayout {
  constexpr explicit MeshLayout() = default;

  /// @p slot - index for vertex buffer
  /// @p size - equivalent of pitch in SDL_gpu, size of consecutive elements of
  /// the vertex buffer, i.e. sizeof(Vertex) if your vertices are of some type
  /// Vertex.
  constexpr MeshLayout &addBinding(Uint32 slot, Uint32 pitch) &;
  constexpr MeshLayout &&addBinding(Uint32 slot, Uint32 pitch) &&;

  constexpr MeshLayout &addAttribute(Uint32 loc, Uint32 binding,
                                     SDL_GPUVertexElementFormat format,
                                     Uint32 offset) &;
  constexpr MeshLayout &&addAttribute(Uint32 loc, Uint32 binding,
                                      SDL_GPUVertexElementFormat format,
                                      Uint32 offset) &&;

  constexpr SDL_GPUVertexInputState toVertexInputState() const;

  constexpr Uint32 numBuffers() const { return _numBuffers; }
  constexpr Uint32 numAttributes() const { return _numAttributes; }

private:
  SDL_GPUVertexBufferDescription vertex_buffer_desc[MAX_VERTEX_BUF_DESCS];
  Uint32 _numBuffers{0};
  SDL_GPUVertexAttribute vertex_attributes[MAX_VERTEX_ATTRS];
  Uint32 _numAttributes{0};
};

constexpr MeshLayout &MeshLayout::addBinding(Uint32 slot, Uint32 pitch) & {
  vertex_buffer_desc[_numBuffers++] = {
      .slot = slot,
      .pitch = pitch,
      .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
      .instance_step_rate = 0,
  };
  return *this;
}

constexpr MeshLayout &&MeshLayout::addBinding(Uint32 slot, Uint32 pitch) && {
  return std::move(addBinding(slot, pitch));
}

constexpr MeshLayout &
MeshLayout::addAttribute(Uint32 loc, Uint32 binding,
                         SDL_GPUVertexElementFormat format, Uint32 offset) & {
  vertex_attributes[_numAttributes++] = {
      .location = loc,
      .buffer_slot = binding,
      .format = format,
      .offset = offset,
  };
  return *this;
}

constexpr MeshLayout &&
MeshLayout::addAttribute(Uint32 loc, Uint32 binding,
                         SDL_GPUVertexElementFormat format, Uint32 offset) && {
  return std::move(addAttribute(loc, binding, format, offset));
}

constexpr SDL_GPUVertexInputState MeshLayout::toVertexInputState() const {
  return {vertex_buffer_desc, _numBuffers, vertex_attributes, _numAttributes};
}

template <typename V>
concept IsVertexType = std::is_standard_layout_v<V>;

template <IsVertexType V> struct VertexTraits;

/// \brief Shortcut for extracting layout from compile-time struct.
template <IsVertexType V> constexpr MeshLayout meshLayoutFor() {
  return VertexTraits<V>::layout();
}

} // namespace candlewick
