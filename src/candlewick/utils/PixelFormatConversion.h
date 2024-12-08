#pragma once

#include <SDL3/SDL_stdinc.h>

namespace candlewick {

/// \brief Convert from 8-bit BGRA to 8-bit RGBA.
void bgraToRgbaConvert(const Uint32 *bgraPixels, Uint32 *rgbaPixels,
                       Uint32 pixelCount);

} // namespace candlewick
