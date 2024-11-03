#pragma once

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_assert.h>

#define CDW_UNREACHABLE_ASSERT(msg)                                            \
  SDL_assert(true);                                                            \
  SDL_LogError(SDL_LOG_CATEGORY_GPU, msg);
