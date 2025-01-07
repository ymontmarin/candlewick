#include "errors.h"
#include <sstream>

namespace candlewick {
std::string RAIIException::error_string(const char *upstreamMsg) {
  std::ostringstream ss;
  ss << "RAIIException: SDL error was";
  ss << upstreamMsg;
  return ss.str();
}
} // namespace candlewick
