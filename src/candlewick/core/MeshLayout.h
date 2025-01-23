#pragma once

#include <SDL3/SDL_gpu.h>
#include <string_view>
#include <vector>

namespace candlewick {

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

/// \brief Struct which defines the layout of a mesh's vertices.
///
/// This is used to build both rasterization pipeline create a Mesh.
/// \sa Mesh
/// \sa MeshData
struct MeshLayout {
  explicit MeshLayout() {}

  /// \brief Add a binding (i.e. a vertex binding) for the mesh.
  ///
  /// Calling this multiple times allows to describe a mesh layout where e.g.
  /// positions and normals are non-interleaved and should be uploaded to
  /// different vertex buffers.
  /// \param slot Index for vertex buffer
  /// \param size Equivalent of pitch in SDL_gpu, size of consecutive elements
  /// of the vertex buffer, i.e. sizeof(Vertex) if your vertices are of some
  /// type Vertex.
  MeshLayout &addBinding(Uint32 slot, Uint32 pitch) &;
  /// \copydoc addBinding()
  MeshLayout &&addBinding(Uint32 slot, Uint32 pitch) &&;

  /// \brief Add a vertex attribute.
  /// \param name The vertex attribute name.
  /// \param loc Location index in the vertex shader.
  /// \param binding Binding slot of the corresponding vertex buffer.
  /// \param format Format of the vertex attribute.
  /// \param offset Byte offset of the attribute relative to the start of the
  /// vertex buffer element.
  MeshLayout &addAttribute(std::string_view name, Uint32 loc, Uint32 binding,
                           SDL_GPUVertexElementFormat format, Uint32 offset) &;
  /// \copydoc addAttribute()
  MeshLayout &&addAttribute(std::string_view name, Uint32 loc, Uint32 binding,
                            SDL_GPUVertexElementFormat format,
                            Uint32 offset) &&;

  /// \brief Cast to the SDL_GPU vertex input state struct, used to create
  /// pipelines.
  /// \warning The data here only references data internal to MeshLayout and
  /// *not* copied. It must stay in scope until the pipeline is created.
  SDL_GPUVertexInputState toVertexInputState() const {
    return {vertex_buffer_desc.data(), numBuffers(), vertex_attributes.data(),
            numAttributes()};
  }

  bool operator==(const MeshLayout &other) const;

  /// \brief Number of vertex buffers.
  Uint32 numBuffers() const { return Uint32(vertex_buffer_desc.size()); }
  /// \brief Number of vertex attributes.
  Uint32 numAttributes() const { return Uint32(vertex_attributes.size()); }
  /// \brief Total size of a vertex (in bytes).
  /// \todo Make this compatible with multiple vertex bindings.
  Uint32 vertexSize() const { return _totalVertexSize; }
  /// \brief Size of mesh indices (in bytes).
  Uint32 indexSize() const { return sizeof(Uint32); }

  const SDL_GPUVertexAttribute *
  lookupAttributeByName(std::string_view name) const {
    for (Uint64 i = 0; i < numAttributes(); i++) {
      if (vertex_attr_names[i] == name)
        return &vertex_attributes[i];
    }
    return NULL;
  }

  MeshLayout &reserve(Uint64 num_buffers, Uint64 num_attributes);

  std::vector<SDL_GPUVertexBufferDescription> vertex_buffer_desc;
  std::vector<SDL_GPUVertexAttribute> vertex_attributes;
  std::vector<std::string_view> vertex_attr_names;

private:
  Uint32 _totalVertexSize{0};
};

/// \brief Validation function. Checks if a MeshLayout produces invalid data for
/// a Mesh.
inline bool validateMeshLayout(const MeshLayout &layout) {
  SDL_GPUVertexInputState inputState = layout.toVertexInputState();
  return (inputState.num_vertex_buffers > 0) &&
         (inputState.vertex_buffer_descriptions) &&
         (inputState.num_vertex_attributes > 0) &&
         (inputState.num_vertex_attributes);
}

/// \brief Basic concept checking if type V has the correct layout and alignment
/// requirements to be a vertex element.
template <typename V>
concept IsVertexType = std::is_standard_layout_v<V> && (alignof(V) == 16);

template <IsVertexType V> struct VertexTraits;

/// \brief Shortcut for extracting layout from compile-time struct.
template <IsVertexType V> MeshLayout meshLayoutFor() {
  return VertexTraits<V>::layout();
}

} // namespace candlewick
