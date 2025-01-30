#pragma once

#include "Scene.h"
#include "Mesh.h"
#include "Renderer.h"
#include "math_types.h"

#include <optional>
#include <entt/entity/registry.hpp>
#include <entt/signal/sigh.hpp>

namespace candlewick {

enum class DebugPipelines {
  TRIANGLE_FILL,
  LINE,
};

class DebugScene;

/// \brief A subsystem for the DebugScene.
///
/// Provides methods for updating debug entities.
/// \sa DebugScene
struct IDebugSubSystem {
  virtual void update(DebugScene & /*scene*/) {};
  virtual void render(const DebugScene & /*scene*/, Renderer & /*renderer*/,
                      SDL_GPURenderPass * /*render_pass*/,
                      const Camera & /*camera*/) {}
  virtual ~IDebugSubSystem() = default;
};

struct DebugScaleComponent {
  Float3 value;
};

struct DebugMeshComponent {
  DebugPipelines pipeline_type;
  Mesh mesh;
  // fragment shader
  std::vector<GpuVec4> colors;
  Mat4f transform;
  bool enable = true;
};

/// \brief %Scene for organizing debug entities and render systems.
///
/// This implements a basic render system for DebugMeshComponent.
class DebugScene final {
  const Device &_device;
  SDL_GPUGraphicsPipeline *_trianglePipeline;
  SDL_GPUGraphicsPipeline *_linePipeline;
  SDL_GPUTextureFormat _swapchainTextureFormat, _depthFormat;
  std::vector<entt::scoped_connection> _connections;
  std::vector<std::unique_ptr<IDebugSubSystem>> _systems;
  entt::registry _registry;

  void onDestroyMeshComponent(entt::registry &registry, entt::entity);

  void renderMeshComponents(Renderer &renderer, SDL_GPURenderPass *render_pass,
                            const Camera &camera) const;

public:
  enum { TRANSFORM_SLOT = 0 };
  enum { COLOR_SLOT = 0 };

  DebugScene(const Renderer &renderer);
  DebugScene(const DebugScene &) = delete;
  DebugScene &operator=(const DebugScene &) = delete;

  const Device &device() const noexcept { return _device; }
  entt::registry &registry() { return _registry; }
  const entt::registry &registry() const { return _registry; }

  /// \brief Add a subsystem (IDebugSubSystem) to the scene.
  template <std::derived_from<IDebugSubSystem> System, typename... Args>
  System &addSystem(Args &&...args) {
    auto sys = std::make_unique<System>(std::forward<Args>(args)...);
    _systems.push_back(std::move(sys));
    return static_cast<System &>(*_systems.back());
  }

  /// \brief Setup pipelines; this will only have an effect **ONCE**.
  void setupPipelines(const MeshLayout &layout);

  /// \brief Just the basic 3D triad.
  std::tuple<entt::entity, DebugMeshComponent &> addTriad();
  /// \brief Add a basic line grid.
  std::tuple<entt::entity, DebugMeshComponent &>
  addLineGrid(std::optional<Float4> color = std::nullopt);

  void update() {
    for (auto &system : _systems) {
      system->update(*this);
    }
  }

  void render(Renderer &renderer, const Camera &camera) const;

  void release();

  ~DebugScene() { release(); }
};

static_assert(Scene<DebugScene>);

} // namespace candlewick
