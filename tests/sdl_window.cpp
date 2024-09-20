#include "candlewick/core/Shader.h"
#include "SDL_gpu_shadercross.h"
#include <SDL3/SDL_gpu.h>

void quit(SDL_GPUDevice *device, SDL_Window *window) {

  SDL_ReleaseWindowFromGPUDevice(device, window);
  SDL_DestroyWindow(window);
  SDL_DestroyGPUDevice(device);
}

int main() {

  bool init_ok = SDL_Init(SDL_INIT_VIDEO);
  if (!init_ok) {
    SDL_Log("Failed to init SDL: %s", SDL_GetError());
  }

  SDL_WindowFlags a;
  const char* title = "sdl_window_test";
  SDL_GPUDevice *device = SDL_CreateGPUDevice(SDL_ShaderCross_GetSPIRVShaderFormats(), true, NULL);
  if (!device) {
    SDL_Log("CreateGPUDevice failed");
    return -1.;
  }
  SDL_Window* window = SDL_CreateWindow(title, 1600, 1200, a);
  if (!window) {
    SDL_Log("Failed to create window! %s", SDL_GetError());
    return -1;
  }

  SDL_ReleaseWindowFromGPUDevice(device, window);
  SDL_DestroyWindow(window);
  SDL_DestroyGPUDevice(device);
  return 0;
}
