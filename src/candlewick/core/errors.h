#pragma once

#include "unreachable.h"

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_assert.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <format>

namespace candlewick {
struct RAIIException : std::runtime_error {
  RAIIException(std::string_view msg);
};

struct InvalidArgument : std::invalid_argument {
  InvalidArgument(const std::string &msg) : std::invalid_argument(msg) {}
};

namespace detail {

  std::string _error_message_impl(const char *fname, std::string_view fmtstr,
                                  std::format_args args);

  template <typename... Ts>
  std::string error_message_format(std::string_view fname,
                                   std::string_view _fmtstr, Ts &&...args) {
    return _error_message_impl(
        fname.data(), _fmtstr,
        std::make_format_args(std::forward<Ts>(args)...));
  }

} // namespace detail
} // namespace candlewick

#define CANDLEWICK_UNREACHABLE_ASSERT(msg)                                     \
  SDL_LogError(                                                                \
      SDL_LOG_CATEGORY_APPLICATION, "%s",                                      \
      ::candlewick::detail::error_message_format(__FUNCTION__, "{:s}", msg)    \
          .c_str());                                                           \
  ::candlewick::unreachable();

#define CANDLEWICK_TERMINATE(msg)                                              \
  SDL_LogError(                                                                \
      SDL_LOG_CATEGORY_APPLICATION, "%s",                                      \
      ::candlewick::detail::error_message_format(__FUNCTION__, "{:s}", msg)    \
          .c_str());                                                           \
  ::std::terminate();
