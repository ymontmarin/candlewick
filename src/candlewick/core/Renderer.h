#pragma once

#include "Core.h"
#include "Device.h"
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

  void beginFrame() { command_buffer = SDL_AcquireGPUCommandBuffer(device); }

  void endFrame() { SDL_SubmitGPUCommandBuffer(command_buffer); }

  void acquireSwapchain() {
    SDL_AcquireGPUSwapchainTexture(command_buffer, window, &swapchain, NULL,
                                   NULL);
  }

  bool hasDepthTexture() const { return depth_texture != nullptr; }

  void destroy() {
    SDL_ReleaseWindowFromGPUDevice(device, window);
    device.destroy();
  }
};

} // namespace candlewick
