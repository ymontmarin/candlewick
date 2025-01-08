#pragma once

#include "Device.h"
#include <SDL3/SDL_gpu.h>

namespace candlewick {

struct Renderer {
  Device device;
  SDL_Window *window = nullptr;
  SDL_GPUTexture *swapchain;
  SDL_GPUCommandBuffer *command_buffer;

  Renderer(Device &&device, SDL_Window *window);

  void beginFrame() { command_buffer = SDL_AcquireGPUCommandBuffer(device); }

  void endFrame() { SDL_SubmitGPUCommandBuffer(command_buffer); }

  ~Renderer() {
    SDL_ReleaseWindowFromGPUDevice(device, window);
    device.destroy();
  }
};

} // namespace candlewick
