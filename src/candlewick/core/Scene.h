#pragma once

#include "Shape.h"
#include "Renderer.h"

namespace candlewick {

template <typename T>
concept Scene = requires(T t, Renderer &r) {
  { t.render(r) } -> std::same_as<void>;
};

} // namespace candlewick
