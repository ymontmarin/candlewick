#pragma once

#include "../DebugScene.h"
#include "Frustum.h"
#include "../AABB.h"
#include "../OBB.h"
#include <variant>

namespace candlewick {

struct DebugFrustumComponent {
  Camera const *otherCam;
};

struct DebugBoundsComponent {
  std::variant<AABB, OBB> bounds;
};

class FrustumBoundsDebugSystem final : IDebugSubSystem {
  FrustumAndBoundsDebug _inner;

public:
  FrustumBoundsDebugSystem(const Renderer &renderer)
      : IDebugSubSystem(), _inner(renderer) {}

  std::tuple<entt::entity, DebugFrustumComponent &>
  addFrustum(DebugScene &scene, const Camera &otherCam) {
    auto &reg = scene.registry();
    auto entity = reg.create();
    return {entity, reg.emplace<DebugFrustumComponent>(entity, &otherCam)};
  }

  template <typename BoundsType>
  std::tuple<entt::entity, DebugBoundsComponent &>
  addBounds(DebugScene &scene, const BoundsType &bounds) {
    auto &reg = scene.registry();
    auto entity = reg.create();
    auto &item = reg.emplace<DebugBoundsComponent>(entity, bounds);
    return {entity, item};
  }

  void render(const DebugScene &scene, Renderer &renderer,
              SDL_GPURenderPass *render_pass, const Camera &camera) override;
};

} // namespace candlewick
