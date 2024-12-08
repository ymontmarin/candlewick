#pragma once

#include "../core/Core.h"
#include "VideoRecorder.h"
#include <SDL3/SDL_gpu.h>

namespace candlewick::media {

void dumpTextureImgToFile(const Device &device, SDL_GPUTexture *texture,
                          SDL_GPUTextureFormat format, const Uint32 width,
                          const Uint32 height, const char *filename);

void videoWriteTextureToFrame(const Device &device, VideoRecorder &recorder,
                              SDL_GPUTexture *texture,
                              SDL_GPUTextureFormat format, const Uint32 width,
                              const Uint32 height);

} // namespace candlewick::media
