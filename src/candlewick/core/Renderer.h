#pragma once

#include "Core.h"
#include "Device.h"
#include "Shape.h"

#include <SDL3/SDL_gpu.h>

namespace candlewick {

struct Renderer {
  Device device;
  SDL_Window *window = nullptr;
  SDL_GPUTexture *swapchain;
  SDL_GPUTexture *depth_texture = nullptr;
  SDL_GPUTextureFormat depth_format;
  SDL_GPUCommandBuffer *command_buffer;

  Renderer(Device &&device, SDL_Window *window);
  Renderer(Device &&device, SDL_Window *window,
           SDL_GPUTextureFormat depth_tex_format);

  /// Acquire the command buffer, starting a frame.
  void beginFrame() { command_buffer = SDL_AcquireGPUCommandBuffer(device); }

  /// Submit the command buffer, ending the frame.
  void endFrame() { SDL_SubmitGPUCommandBuffer(command_buffer); }

  bool acquireSwapchain() {
    return SDL_AcquireGPUSwapchainTexture(command_buffer, window, &swapchain,
                                          NULL, NULL);
  }

  bool hasDepthTexture() const { return depth_texture != nullptr; }

  /// Render an individual mesh as part of a render pass, using a provided first
  /// index or vertex offset.
  /// \details This routine will bind the vertex and index buffer. Prefer one of
  /// the other routines to e.g. batch draw calls that use the same buffer
  /// bindings.
  void render(SDL_GPURenderPass *pass, const Mesh &mesh,
              Uint32 firstIndexOrVertex = 0);

  /// Render a collection of \c Mesh collected in a \c Shape object, which
  /// satisfies the necessary invariants to allow for batching. We only have to
  /// bind the vertex and index buffers once, here.
  /// \overload render()
  void render(SDL_GPURenderPass *pass, const Shape &shape);

  void pushVertexUniform(Uint32 slot_index, const void *data, Uint32 length) {
    SDL_PushGPUVertexUniformData(command_buffer, slot_index, data, length);
  }
  void pushFragmentUniform(Uint32 slot_index, const void *data, Uint32 length) {
    SDL_PushGPUFragmentUniformData(command_buffer, slot_index, data, length);
  }

  void destroy() {
    SDL_ReleaseWindowFromGPUDevice(device, window);
    device.destroy();
  }
};

} // namespace candlewick
