#include "errors.h"
#include <format>

namespace candlewick {
std::string RAIIException::error_string(std::string_view upstreamMsg) {
  return std::format("RAIIException: SDL error \'%s\'", upstreamMsg);
}
} // namespace candlewick
