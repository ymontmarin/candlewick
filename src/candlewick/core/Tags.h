#pragma once

namespace candlewick {

struct NoInitT {
  constexpr explicit NoInitT() {}
};
constexpr NoInitT NoInit{};

} // namespace candlewick
