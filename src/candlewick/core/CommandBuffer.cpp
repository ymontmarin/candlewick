#include "CommandBuffer.h"
#include "Device.h"

namespace candlewick {

CommandBuffer::CommandBuffer(const Device &device) {
  _cmdBuf = SDL_AcquireGPUCommandBuffer(device);
}

} // namespace candlewick
