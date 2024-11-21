#include "candlewick/core/Device.h"

#include <SDL3/SDL_gpu.h>

using candlewick::Device;
using candlewick::NoInit;

const float kScrollZoom = 0.05;

struct Context {
  Device device{NoInit};
  SDL_Window *window;
};

bool ExampleInit(Context &ctx, float wWidth, float wHeight);
void ExampleTeardown(Context &ctx);
