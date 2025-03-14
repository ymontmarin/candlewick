#include "errors.h"
#include <format>

namespace candlewick {
RAIIException::RAIIException(std::string_view msg)
    : std::runtime_error(
          std::format("RAIIException: SDL error \'{}\'", msg.data())) {}

namespace detail {
  std::string _error_message_impl(const char *fname, std::string_view _fmtstr,
                                  std::format_args args) {
    return std::format("{:s} :: {:s}", fname, std::vformat(_fmtstr, args));
  }
} // namespace detail
} // namespace candlewick
