#include "errors.h"
#include <format>

namespace candlewick {
std::string RAIIException::error_string(const char *upstreamMsg) {
  return std::format("RAIIException: SDL error \'%s\'", upstreamMsg);
}
} // namespace candlewick
