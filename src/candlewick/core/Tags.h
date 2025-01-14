#pragma once

namespace candlewick {

/// \brief Tag type for non-initializing constructors (for e.g. RAII classes)
struct NoInitT {
  constexpr explicit NoInitT() {}
};
constexpr NoInitT NoInit{};

} // namespace candlewick
