#pragma once

#include "Core.h"
#include <SDL3/SDL_gpu.h>
#include <cassert>

namespace candlewick {

class CommandBuffer {
  SDL_GPUCommandBuffer *_cmdBuf;
  CommandBuffer(SDL_GPUCommandBuffer *ptr) : _cmdBuf{ptr} {}

public:
  static CommandBuffer acquire(const Device &device);
  operator SDL_GPUCommandBuffer *() const { return _cmdBuf; }

  CommandBuffer(const CommandBuffer &) = delete;
  CommandBuffer(CommandBuffer &&) = delete;

  void submit() { SDL_SubmitGPUCommandBuffer(_cmdBuf); }
  void cancel() { SDL_CancelGPUCommandBuffer(_cmdBuf); }

  /// \brief Push uniform data to the vertex shader.
  CommandBuffer &pushVertexUniform(Uint32 slot_index, const void *data,
                                   Uint32 length) {
    SDL_PushGPUVertexUniformData(_cmdBuf, slot_index, data, length);
    return *this;
  }
  /// \brief Push uniform data to the fragment shader.
  CommandBuffer &pushFragmentUniform(Uint32 slot_index, const void *data,
                                     Uint32 length) {
    SDL_PushGPUFragmentUniformData(_cmdBuf, slot_index, data, length);
    return *this;
  }

  SDL_GPUFence *submitAndAcquireFence() {
    return SDL_SubmitGPUCommandBufferAndAcquireFence(_cmdBuf);
  }
};

} // namespace candlewick
