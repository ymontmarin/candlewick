#pragma once
#include "imgui_impl_sdl3.h"

struct SDL_GPUDevice;

IMGUI_IMPL_API bool ImGui_ImplSDL3_InitForSDLGPU3(SDL_Window *window,
                                                  SDL_GPUDevice *device);
