#include "CommandBuffer.h"
#include "Device.h"

namespace candlewick {

CommandBuffer::CommandBuffer(const Device &device) {
  _cmdBuf = SDL_AcquireGPUCommandBuffer(device);
}

CommandBuffer &CommandBuffer::operator=(CommandBuffer &&other) noexcept {
  if (active()) {
    this->cancel();
  }
  _cmdBuf = other._cmdBuf;
  other._cmdBuf = nullptr;
  return *this;
}

} // namespace candlewick
