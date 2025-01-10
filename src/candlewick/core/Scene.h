#pragma once

#include "Renderer.h"

namespace candlewick {

template <typename T>
concept Scene = requires(T t, Renderer &r) {
  { t.render(r) } -> std::same_as<void>;
  { t.release() } -> std::same_as<void>;
};

} // namespace candlewick
