#include "Renderer.h"
#include "errors.h"
#include <utility>
#include "../third-party/magic_enum.hpp"

namespace candlewick {
Renderer::Renderer(Device &&device, SDL_Window *window)
    : device(std::move(device)), window(window) {
  if (!SDL_ClaimWindowForGPUDevice(this->device, this->window))
    throw RAIIException(SDL_GetError());
}

static bool texture_create_info_supported(SDL_GPUDevice *device,
                                          SDL_GPUTextureCreateInfo info) {
  return SDL_GPUTextureSupportsFormat(device, info.format, info.type,
                                      info.usage);
}

Renderer::Renderer(Device &&device, SDL_Window *window,
                   SDL_GPUTextureFormat suggested_depth_format)
    : Renderer(std::move(device), window) {

  int width, height;
  SDL_GetWindowSize(window, &width, &height);

  SDL_GPUTextureCreateInfo texInfo{
      .type = SDL_GPU_TEXTURETYPE_2D,
      .format = suggested_depth_format,
      .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
      .width = Uint32(width),
      .height = Uint32(height),
      .layer_count_or_depth = 1,
      .num_levels = 1,
      .sample_count = SDL_GPU_SAMPLECOUNT_1,
  };
  SDL_GPUTextureFormat depth_format_fallbacks[] = {
      SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT,
      SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
      SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT,
  };
  size_t try_idx = 0;
  while (!texture_create_info_supported(this->device, texInfo) &&
         try_idx < std::size(depth_format_fallbacks)) {
    texInfo.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
    try_idx++;
  }
  this->depth_format = texInfo.format;
  SDL_Log("[Renderer] depth texture format will be %s",
          magic_enum::enum_name(this->depth_format).data());
  depth_texture = SDL_CreateGPUTexture(this->device, &texInfo);
}

void Renderer::bindMesh(SDL_GPURenderPass *pass, const Mesh &mesh) {
  const MeshLayout &layout = mesh.layout;
  std::vector<SDL_GPUBufferBinding> vertex_bindings;
  vertex_bindings.reserve(layout.numBuffers());
  for (Uint32 j = 0; j < layout.numBuffers(); j++) {
    vertex_bindings.push_back(mesh.getVertexBinding(j));
  }

  SDL_BindGPUVertexBuffers(pass, 0, vertex_bindings.data(),
                           layout.numBuffers());
  if (mesh.isIndexed()) {
    SDL_GPUBufferBinding index_binding = mesh.getIndexBinding();
    SDL_BindGPUIndexBuffer(pass, &index_binding,
                           SDL_GPU_INDEXELEMENTSIZE_32BIT);
  }
}

void Renderer::drawView(SDL_GPURenderPass *pass, const MeshView &mesh) {
  assert(validateMeshView(mesh));
  if (mesh.isIndexed()) {
    SDL_DrawGPUIndexedPrimitives(pass, mesh.indexCount, 1, mesh.indexOffset,
                                 Sint32(mesh.vertexOffsets[0]), 0);
  } else {
    SDL_DrawGPUPrimitives(pass, mesh.vertexCount, 1, mesh.vertexOffsets[0], 0);
  }
}

void Renderer::drawViews(SDL_GPURenderPass *pass,
                         std::span<const MeshView> meshViews) {
  if (meshViews.empty())
    return;

#ifndef NDEBUG
  const auto ib = meshViews[0].indexBuffer;
  const auto &vbs = meshViews[0].vertexBuffers;
  const auto n_vbs = vbs.size();
#endif
  for (auto &view : meshViews) {
#ifndef NDEBUG
    SDL_assert(ib == view.indexBuffer);
    for (size_t i = 0; i < n_vbs; i++)
      SDL_assert(vbs[i] == view.vertexBuffers[i]);
#endif
    this->drawView(pass, view);
  }
}

// void Renderer::render(SDL_GPURenderPass *pass, const Shape &shape) {
//   if (shape.meshes().empty())
//     return;

//   if (shape.layout().numBuffers() != 1) {
//     throw InvalidArgument("renderShape() only supports meshes with a "
//                           "single-buffer vertex layout.");
//   }
//   SDL_GPUBufferBinding vertex_binding = shape.getVertexBinding();
//   SDL_GPUBufferBinding index_binding = shape.getIndexBinding();

//   SDL_BindGPUVertexBuffers(pass, 0, &vertex_binding, 1);
//   SDL_BindGPUIndexBuffer(pass, &index_binding,
//   SDL_GPU_INDEXELEMENTSIZE_32BIT);

//   Uint32 indexOffset = 0;
//   Uint32 vertexOffset = 0;
//   for (size_t j = 0; j < shape.meshes().size(); j++) {
//     const auto &mesh = shape.meshes()[j];
//     auto materialUniform = shape.materials()[j].toUniform();
//     SDL_PushGPUFragmentUniformData(command_buffer, Shape::MATERIAL_SLOT,
//                                    &materialUniform,
//                                    sizeof(materialUniform));
//     if (mesh.isIndexed())
//       SDL_DrawGPUIndexedPrimitives(pass, mesh.count, 1, indexOffset,
//                                    Sint32(vertexOffset), 0);
//     else
//       SDL_DrawGPUPrimitives(pass, mesh.count, 1, vertexOffset, 0);
//     indexOffset += mesh.indexCount;
//     vertexOffset += mesh.vertexCount;
//   }
// }

} // namespace candlewick
