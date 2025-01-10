#pragma once

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_assert.h>
#include <stdexcept>

#define CDW_UNREACHABLE_ASSERT(msg)                                            \
  SDL_assert(true);                                                            \
  SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s: %s", __FUNCTION__, msg);

namespace candlewick {
class RAIIException : public std::runtime_error {
  static std::string error_string(std::string_view upstreamMsg);

public:
  RAIIException(std::string_view upstreamMsg)
      : std::runtime_error(error_string(upstreamMsg)) {}
};

class InvalidArgument : public std::invalid_argument {
public:
  InvalidArgument(const std::string &msg) : std::invalid_argument(msg) {}
};

} // namespace candlewick
