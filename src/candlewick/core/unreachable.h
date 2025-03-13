#pragma once

#if (__cpp_lib_unreachable >= 202202L)
#include <utility>
#endif

namespace candlewick {

[[noreturn]] inline void unreachable() {
#if (__cpp_lib_unreachable >= 202202L)
  std::unreachable();
#elif defined(_MSC_VER)
  __assume(false);
#else
  __builtin_unreachable();
#endif
}

} // namespace candlewick
