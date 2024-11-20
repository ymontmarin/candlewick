#include "Common.h"
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <new>

bool ExampleInit(Context &ctx, float wWidth, float wHeight) {
  if (!SDL_Init(SDL_INIT_VIDEO))
    return false;
  new (&ctx.device) Device{SDL_GPU_SHADERFORMAT_SPIRV, true};

  ctx.window = SDL_CreateWindow(__FILE_NAME__, wWidth, wHeight, 0);
  if (!SDL_ClaimWindowForGPUDevice(ctx.device, ctx.window)) {
    SDL_Log("Error %s", SDL_GetError());
    return false;
  }

  return true;
}

void ExampleTeardown(Context &ctx) {
  SDL_ReleaseWindowFromGPUDevice(ctx.device, ctx.window);
  SDL_DestroyWindow(ctx.window);
  ctx.device.destroy();
  SDL_Quit();
}
