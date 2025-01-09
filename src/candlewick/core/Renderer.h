#pragma once

#include "Core.h"
#include "Device.h"
#include "Shape.h"
#include "errors.h"

#include <SDL3/SDL_gpu.h>

namespace candlewick {

struct Renderer {
  Device device;
  SDL_Window *window = nullptr;
  SDL_GPUTexture *swapchain;
  SDL_GPUTexture *depth_texture = nullptr;
  SDL_GPUCommandBuffer *command_buffer;

  Renderer(Device &&device, SDL_Window *window);
  Renderer(Device &&device, SDL_Window *window,
           SDL_GPUTextureFormat depth_tex_format);

  /// Acquire the command buffer, starting a frame.
  void beginFrame() { command_buffer = SDL_AcquireGPUCommandBuffer(device); }

  /// Submit the command buffer, ending the frame.
  void endFrame() { SDL_SubmitGPUCommandBuffer(command_buffer); }

  void acquireSwapchain() {
    SDL_AcquireGPUSwapchainTexture(command_buffer, window, &swapchain, NULL,
                                   NULL);
  }

  bool hasDepthTexture() const { return depth_texture != nullptr; }

  /// Render a collection of \c Mesh collected in a \c Shape object, which
  /// satisfies the necessary invariants to allow for batching. We only have to
  /// bind the vertex and index buffers once, here.
  void renderShape(SDL_GPURenderPass *pass, const Shape &shape) {
    if (shape.meshes().empty())
      return;

    if (shape.layout().numBuffers() != 1) {
      throw InvalidArgument("renderShape() only supports meshes with a "
                            "single-buffer vertex layout.");
    }
    SDL_GPUBufferBinding vertex_binding = shape.getVertexBinding();
    SDL_GPUBufferBinding index_binding = shape.getIndexBinding();

    SDL_BindGPUVertexBuffers(pass, 0, &vertex_binding, 1);
    SDL_BindGPUIndexBuffer(pass, &index_binding,
                           SDL_GPU_INDEXELEMENTSIZE_32BIT);

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

  void destroy() {
    SDL_ReleaseWindowFromGPUDevice(device, window);
    device.destroy();
  }
};

} // namespace candlewick
