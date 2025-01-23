#include "MeshLayout.h"
#include "math_types.h"
#include <utility>
#include <cstring>

bool operator==(const SDL_GPUVertexBufferDescription &lhs,
                const SDL_GPUVertexBufferDescription &rhs) {
#define _c(field) lhs.field == rhs.field
  return _c(slot) && _c(pitch) && _c(input_rate) && _c(instance_step_rate);
#undef _c
}

bool operator==(const SDL_GPUVertexAttribute &lhs,
                const SDL_GPUVertexAttribute &rhs) {
#define _c(field) lhs.field == rhs.field
  return _c(location) && _c(buffer_slot) && _c(format) && _c(offset);
#undef _c
}

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

MeshLayout &MeshLayout::addAttribute(std::string_view name, Uint32 loc,
                                     Uint32 binding,
                                     SDL_GPUVertexElementFormat format,
                                     Uint32 offset) & {
  vertex_attr_names.push_back(name);
  vertex_attributes.push_back({
      .location = loc,
      .buffer_slot = binding,
      .format = format,
      .offset = offset,
  });
  const Uint32 attrSize = math::roundUpTo16(Uint32(vertexElementSize(format)));
  _totalVertexSize = math::max(_totalVertexSize, offset + attrSize);
  return *this;
}

MeshLayout &&MeshLayout::addAttribute(std::string_view name, Uint32 loc,
                                      Uint32 binding,
                                      SDL_GPUVertexElementFormat format,
                                      Uint32 offset) && {
  return std::move(addAttribute(name, loc, binding, format, offset));
}

bool MeshLayout::operator==(const MeshLayout &other) const {
  if (vertex_attributes != other.vertex_attributes)
    return false;

  return vertex_buffer_desc == other.vertex_buffer_desc;
}

MeshLayout &MeshLayout::reserve(Uint64 num_buffers, Uint64 num_attributes) {
  vertex_buffer_desc.reserve(num_buffers);
  vertex_attr_names.reserve(num_attributes);
  vertex_attributes.reserve(num_attributes);
  return *this;
}

} // namespace candlewick
