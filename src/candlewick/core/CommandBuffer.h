#pragma once

#include "Core.h"
#include <SDL3/SDL_gpu.h>
#include <cassert>
#include <utility>

namespace candlewick {

class CommandBuffer {
  SDL_GPUCommandBuffer *_cmdBuf;

public:
  CommandBuffer(const Device &device);
  operator SDL_GPUCommandBuffer *() const { return _cmdBuf; }

  CommandBuffer(const CommandBuffer &) = delete;
  CommandBuffer(CommandBuffer &&other) noexcept : _cmdBuf(other._cmdBuf) {
    other._cmdBuf = nullptr;
  }

  CommandBuffer &operator=(CommandBuffer &&other) noexcept;

  friend void swap(CommandBuffer &lhs, CommandBuffer &rhs) noexcept {
    std::swap(lhs._cmdBuf, rhs._cmdBuf);
  }

  bool submit() noexcept {
    if (!(active() && SDL_SubmitGPUCommandBuffer(_cmdBuf)))
      return false;
    _cmdBuf = nullptr;
    return true;
  }

  bool cancel() noexcept {
    if (!(active() && SDL_CancelGPUCommandBuffer(_cmdBuf)))
      return false;
    _cmdBuf = nullptr;
    return true;
  }

  bool active() const noexcept { return _cmdBuf; }

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
