#include "errors.h"
#include <format>

namespace candlewick {
RAIIException::RAIIException(std::string_view msg)
    : std::runtime_error(
          std::format("RAIIException: SDL error \'{}\'", msg.data())) {}
} // namespace candlewick
