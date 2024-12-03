#pragma once

#include "../core/Core.h"
#include <SDL3/SDL_gpu.h>

namespace candlewick::media {

void dumpTextureImgToFile(const Device &device, SDL_GPUTexture *texture,
                          SDL_GPUTextureFormat format, const Uint32 width,
                          const Uint32 height, const char *filename);

} // namespace candlewick::media
