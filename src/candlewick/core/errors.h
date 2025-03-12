#pragma once

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_assert.h>
#include <stdexcept>
#include <string>
#include <string_view>

#define CDW_UNREACHABLE_ASSERT(msg)                                            \
  SDL_assert(true);                                                            \
  SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s: %s", __FUNCTION__, msg);

namespace candlewick {
class RAIIException : public std::runtime_error {
public:
  RAIIException(std::string_view msg);
};

class InvalidArgument : public std::invalid_argument {
public:
  InvalidArgument(const std::string &msg) : std::invalid_argument(msg) {}
};

} // namespace candlewick
