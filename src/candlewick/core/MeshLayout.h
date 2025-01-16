#pragma once

#include <SDL3/SDL_gpu.h>
#include <utility>
#include <string_view>

namespace candlewick {

namespace math {
template <typename T> constexpr T max(const T &a, const T &b) {
  if (a < b)
    return b;
  return a;
}

constexpr Uint32 roundUpTo16(Uint32 value) {
  Uint32 q = value / 16;
  Uint32 r = value % 16;
  if (r == 0)
    return value;
  return (q + 1) * 16u;
}
} // namespace math

static constexpr Uint32 MAX_VERTEX_BUF_DESCS = 10u;
static constexpr Uint32 MAX_VERTEX_ATTRS = 10u;

constexpr Uint64 vertexElementSize(SDL_GPUVertexElementFormat format) {
  switch (format) {
  case SDL_GPU_VERTEXELEMENTFORMAT_INT:
    return sizeof(Sint32);
  case SDL_GPU_VERTEXELEMENTFORMAT_INT2:
    return sizeof(Sint32[2]);
  case SDL_GPU_VERTEXELEMENTFORMAT_INT3:
    return sizeof(Sint32[3]);
  case SDL_GPU_VERTEXELEMENTFORMAT_INT4:
    return sizeof(Sint32[4]);
  case SDL_GPU_VERTEXELEMENTFORMAT_UINT:
    return sizeof(Uint32);
  case SDL_GPU_VERTEXELEMENTFORMAT_UINT2:
    return sizeof(Uint32[2]);
  case SDL_GPU_VERTEXELEMENTFORMAT_UINT3:
    return sizeof(Uint32[3]);
  case SDL_GPU_VERTEXELEMENTFORMAT_UINT4:
    return sizeof(Uint32[4]);
  case SDL_GPU_VERTEXELEMENTFORMAT_FLOAT:
    return sizeof(float);
  case SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2:
    return sizeof(float[2]);
  case SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3:
    return sizeof(float[3]);
  case SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4:
    return sizeof(float[4]);
  default:
    return 0u;
  }
}

/// Struct which defines the layout of a mesh's vertices.
struct MeshLayout {
  constexpr explicit MeshLayout() = default;

  /// @p slot - index for vertex buffer
  /// @p size - equivalent of pitch in SDL_gpu, size of consecutive elements of
  /// the vertex buffer, i.e. sizeof(Vertex) if your vertices are of some type
  /// Vertex.
  constexpr MeshLayout &addBinding(Uint32 slot, Uint32 pitch) &;
  constexpr MeshLayout &&addBinding(Uint32 slot, Uint32 pitch) &&;

  constexpr MeshLayout &addAttribute(std::string_view name, Uint32 loc,
                                     Uint32 binding,
                                     SDL_GPUVertexElementFormat format,
                                     Uint32 offset) &;
  constexpr MeshLayout &&addAttribute(std::string_view name, Uint32 loc,
                                      Uint32 binding,
                                      SDL_GPUVertexElementFormat format,
                                      Uint32 offset) &&;

  constexpr SDL_GPUVertexInputState toVertexInputState() const;

  /// \brief Number of vertex buffers.
  constexpr Uint32 numBuffers() const { return _numBuffers; }
  /// \brief Number of vertex attributes.
  constexpr Uint32 numAttributes() const { return _numAttributes; }
  constexpr Uint32 vertexSize() const { return _totalVertexSize; }

  const SDL_GPUVertexAttribute *
  lookupAttributeByName(std::string_view name) const {
    for (Uint64 i = 0; i < _numAttributes; i++) {
      if (vertex_attr_names[i] == name)
        return &vertex_attributes[i];
    }
    return NULL;
  }

  SDL_GPUVertexBufferDescription vertex_buffer_desc[MAX_VERTEX_BUF_DESCS];
  SDL_GPUVertexAttribute vertex_attributes[MAX_VERTEX_ATTRS];
  std::string_view vertex_attr_names[MAX_VERTEX_ATTRS];

private:
  Uint32 _numBuffers{0};
  Uint32 _numAttributes{0};
  Uint32 _totalVertexSize{0};
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
MeshLayout::addAttribute(std::string_view name, Uint32 loc, Uint32 binding,
                         SDL_GPUVertexElementFormat format, Uint32 offset) & {
  vertex_attr_names[_numAttributes] = name;
  vertex_attributes[_numAttributes++] = {
      .location = loc,
      .buffer_slot = binding,
      .format = format,
      .offset = offset,
  };
  const Uint32 attrSize = math::roundUpTo16(Uint32(vertexElementSize(format)));
  _totalVertexSize = math::max(_totalVertexSize, offset + attrSize);
  return *this;
}

constexpr MeshLayout &&
MeshLayout::addAttribute(std::string_view name, Uint32 loc, Uint32 binding,
                         SDL_GPUVertexElementFormat format, Uint32 offset) && {
  return std::move(addAttribute(name, loc, binding, format, offset));
}

constexpr SDL_GPUVertexInputState MeshLayout::toVertexInputState() const {
  return {vertex_buffer_desc, _numBuffers, vertex_attributes, _numAttributes};
}

template <typename V>
concept IsVertexType = std::is_standard_layout_v<V> && (alignof(V) == 16);

template <IsVertexType V> struct VertexTraits;

/// \brief Shortcut for extracting layout from compile-time struct.
template <IsVertexType V> constexpr MeshLayout meshLayoutFor() {
  return VertexTraits<V>::layout();
}

} // namespace candlewick
