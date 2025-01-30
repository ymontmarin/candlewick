#include "FrustumDebugSystem.h"

namespace candlewick {

void FrustumBoundsDebugSystem::render(const DebugScene &scene,
                                      Renderer &renderer,
                                      SDL_GPURenderPass *render_pass,
                                      const Camera &camera) {
  auto &reg = scene.registry();

  SDL_BindGPUGraphicsPipeline(render_pass, _inner.pipeline);

  for (auto [ent, item] : reg.view<const DebugFrustumComponent>().each()) {
    _inner.renderFrustum(renderer, camera, *item.otherCam);
  }

  for (auto [ent, item] : reg.view<const DebugBoundsComponent>().each()) {
    std::visit(
        entt::overloaded{
            [&](const AABB &bounds) {
              _inner.renderAABB(renderer, camera, bounds);
            },
            [&](const OBB &obb) { _inner.renderOBB(renderer, camera, obb); },
        },
        item.bounds);
  }
}

} // namespace candlewick
