#include "CommandBuffer.h"
#include "Device.h"

namespace candlewick {

CommandBuffer CommandBuffer::acquire(const Device &device) {
  return CommandBuffer(SDL_AcquireGPUCommandBuffer(device));
}

} // namespace candlewick
