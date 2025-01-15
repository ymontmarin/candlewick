#include "Renderer.h"
#include "errors.h"
#include <utility>

namespace candlewick {
Renderer::Renderer(Device &&device, SDL_Window *window)
    : device(std::move(device)), window(window) {
  if (!SDL_ClaimWindowForGPUDevice(this->device, this->window))
    throw RAIIException(SDL_GetError());
}

Renderer::Renderer(Device &&device, SDL_Window *window,
                   SDL_GPUTextureFormat depth_tex_format)
    : Renderer(std::move(device), window) {

  int width, height;
  SDL_GetWindowSize(window, &width, &height);
  this->depth_format = depth_tex_format;
  SDL_GPUTextureCreateInfo texInfo{
      .type = SDL_GPU_TEXTURETYPE_2D,
      .format = depth_tex_format,
      .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
      .width = Uint32(width),
      .height = Uint32(height),
      .layer_count_or_depth = 1,
      .num_levels = 1,
      .sample_count = SDL_GPU_SAMPLECOUNT_1,
  };
  depth_texture = SDL_CreateGPUTexture(this->device, &texInfo);
}

void Renderer::render(SDL_GPURenderPass *pass, const Mesh &mesh,
                      Uint32 firstIndexOrVertex) {
  const MeshLayout &layout = mesh.layout();
  std::vector<SDL_GPUBufferBinding> vertex_bindings;
  vertex_bindings.reserve(layout.numBuffers());
  for (Uint32 j = 0; j < layout.numBuffers(); j++) {
    vertex_bindings.push_back(mesh.getVertexBinding(j));
  }
  SDL_GPUBufferBinding index_binding = mesh.getIndexBinding();

  SDL_BindGPUVertexBuffers(pass, 0, vertex_bindings.data(),
                           layout.numBuffers());
  SDL_BindGPUIndexBuffer(pass, &index_binding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

  if (mesh.isIndexed()) {
    SDL_DrawGPUIndexedPrimitives(pass, mesh.count, 1, firstIndexOrVertex, 0, 0);
  } else {
    SDL_DrawGPUPrimitives(pass, mesh.count, 1, firstIndexOrVertex, 0);
  }
}

void Renderer::render(SDL_GPURenderPass *pass, const Shape &shape) {
  if (shape.meshes().empty())
    return;

  if (shape.layout().numBuffers() != 1) {
    throw InvalidArgument("renderShape() only supports meshes with a "
                          "single-buffer vertex layout.");
  }
  SDL_GPUBufferBinding vertex_binding = shape.getVertexBinding();
  SDL_GPUBufferBinding index_binding = shape.getIndexBinding();

  SDL_BindGPUVertexBuffers(pass, 0, &vertex_binding, 1);
  SDL_BindGPUIndexBuffer(pass, &index_binding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

  Uint32 indexOffset = 0;
  Uint32 vertexOffset = 0;
  for (size_t j = 0; j < shape.meshes().size(); j++) {
    const auto &mesh = shape.meshes()[j];
    auto materialUniform = shape.materials()[j].toUniform();
    SDL_PushGPUFragmentUniformData(command_buffer, Shape::MATERIAL_SLOT,
                                   &materialUniform, sizeof(materialUniform));
    if (mesh.isIndexed())
      SDL_DrawGPUIndexedPrimitives(pass, mesh.count, 1, indexOffset,
                                   Sint32(vertexOffset), 0);
    else
      SDL_DrawGPUPrimitives(pass, mesh.count, 1, vertexOffset, 0);
    indexOffset += mesh.count;
    vertexOffset += mesh.vertexCount;
  }
}

} // namespace candlewick
