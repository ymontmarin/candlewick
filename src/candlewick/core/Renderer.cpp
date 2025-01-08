#include "Renderer.h"
#include "errors.h"
#include <utility>

namespace candlewick {

Renderer::Renderer(Device &&device, SDL_Window *window)
    : device(std::move(device)), window(window) {
  if (!SDL_ClaimWindowForGPUDevice(this->device, this->window))
    throw RAIIException(SDL_GetError());
}

} // namespace candlewick
