#pragma once

#include "Renderer.h"
#include "../utils/CameraState.h"

namespace candlewick {

struct Camera;

template <typename T>
concept Scene = requires(T t, Renderer &r, const Camera &camera) {
  { t.render(r, camera) } -> std::same_as<void>;
  { t.release() } -> std::same_as<void>;
};

} // namespace candlewick
