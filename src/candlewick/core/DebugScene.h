#pragma once

#include "Scene.h"
#include "Mesh.h"
#include "Renderer.h"
#include "math_types.h"

#include <memory>

namespace candlewick {

enum class DebugPipelines {
  TRIANGLE_FILL,
  LINE,
};

struct DebugDrawCommand {
  DebugPipelines pipeline_type;
  const Mesh *mesh;
  std::vector<MeshView> mesh_views;
  // uniform data :: matches our common debug shader
  // vertex
  GpuMat4 mvp;
  // fragment
  std::vector<GpuVec4> colors;
};

class DebugScene;

struct DebugModule {
  virtual void addDrawCommands(DebugScene &scene, const Camera &camera) = 0;
  virtual ~DebugModule() = default;
  DebugModule(DebugScene & /* scene */) {}
};

/// \brief Just the basic 3D triad, and a line grid.
struct BasicDebugModule final : DebugModule {
  Mesh triad;
  std::vector<MeshView> triad_views;
  std::array<GpuVec4, 3> triad_colors;
  Mesh grid;
  GpuVec4 grid_color;

  /// Flag for enabling the 3D triad (the coordinate frame at the origin)
  bool enableTriad{true};
  /// Flag for enabling the wireframe grid
  bool enableGrid{true};

  BasicDebugModule(DebugScene &scene);
  void addDrawCommands(DebugScene &scene, const Camera &camera) override;
};

/// \brief A render system for Debug elements (coordinate frames, arrows,
/// grids...)
///
/// \todo Reorganize the render pipelines and how they are set up.
class DebugScene {
  std::vector<std::unique_ptr<DebugModule>> modules;
  const Device &_device;
  SDL_GPUGraphicsPipeline *trianglePipeline;
  SDL_GPUGraphicsPipeline *linePipeline;
  SDL_GPUTextureFormat _swapchainTextureFormat, _depthFormat;

public:
  enum { TRANSFORM_SLOT = 0 };
  enum { COLOR_SLOT = 0 };
  std::vector<DebugDrawCommand> drawCommands;

  DebugScene(const Renderer &renderer);

  const auto &device() const { return _device; }

  /// \brief Setup pipelines; this will only have an effect **ONCE**.
  void setupPipelines(const MeshLayout &layout);

  /// \brief Add a debug module (inheriting from DebugModule).
  ///
  /// This will inject \c this into the module's constructor.
  /// \tparam U The type of the debug module to be constructed and added.
  /// \param ctor_args Constructor arguments (other than \c *this) for U
  /// \returns Reference to the concrete type U
  template <std::derived_from<DebugModule> U, typename... CtorArgs>
  U &addModule(CtorArgs &&...ctor_args) {
    U *p = new U(*this, std::forward<CtorArgs>(ctor_args)...);
    modules.emplace_back(p);
    return *p;
  }

  void render(Renderer &renderer, const Camera &camera);

  void release();
};

static_assert(Scene<DebugScene>);

} // namespace candlewick
