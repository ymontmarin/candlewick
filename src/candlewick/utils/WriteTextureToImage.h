#pragma once

#include "../core/Core.h"
#ifdef CANDLEWICK_WITH_FFMPEG_SUPPORT
#include "VideoRecorder.h"
#endif
#include <SDL3/SDL_gpu.h>

namespace candlewick {
namespace media {

  void dumpTextureImgToFile(const Device &device, SDL_GPUTexture *texture,
                            SDL_GPUTextureFormat format, const Uint32 width,
                            const Uint32 height, const char *filename);

#ifdef CANDLEWICK_WITH_FFMPEG_SUPPORT
  void videoWriteTextureToFrame(const Device &device, VideoRecorder &recorder,
                                SDL_GPUTexture *texture,
                                SDL_GPUTextureFormat format, const Uint32 width,
                                const Uint32 height);
#endif

} // namespace media
} // namespace candlewick
