#include "errors.h"
#include <format>

namespace candlewick {
RAIIException::RAIIException(std::string_view upstreamMsg)
    : std::runtime_error(
          std::format("RAIIException: SDL error \'%s\'", upstreamMsg.data())) {}
} // namespace candlewick
