/// \defgroup depth_pass Depth Pre-Pass System
/// \{
///
/// \brief Requirements for consistent depth testing between passes.
///
/// When using a depth pre-pass with `EQUAL` depth comparison in the main pass,
/// ensure identical vertex transformations between passes by:
/// 1. Computing the MVP matrix on CPU side
/// 2. Using the same MVP matrix in both pre-pass and main pass shaders
/// 3. Avoiding shader-side matrix multiplication that might cause precision
/// differences.
///
/// Failing to do this can result in z-fighting/Moiré patterns due to
/// floating-point precision differences between CPU and GPU matrix
/// calculations.
///
/// \image html depth-prepass.png "Rendering the depth buffer after early pass"
///
/// \}
#pragma once

#include "Core.h"
#include "Mesh.h"
#include "math_types.h"
#include "LightUniforms.h"

#include <span>

namespace candlewick {

/// \ingroup depth_pass
/// \brief Helper struct for depth or light pre-passes.
/// \warning The depth pre-pass is meant for _opaque_ geometries. Hence, the
/// pipeline created by the create() factory function will have back-face
/// culling enabled.
struct DepthPassInfo {
  enum DepthPassSlots : Uint32 {
    TRANSFORM_SLOT = 0,
  };
  SDL_GPUTexture *depthTexture;
  SDL_GPUGraphicsPipeline *pipeline;

  /// \brief Create DepthPass (e.g. for a depth pre-pass) from a Renderer
  /// and specified MeshLayout.
  ///
  /// \param renderer The Renderer object.
  /// \param layout %Mesh layout used for the render pipeline's vertex input
  /// state.
  /// \param depth_texture If NULL, we assume that the depth texture to use is
  /// the shared depth texture stored in \p renderer.
  /// \sa createShadowPass()
  static DepthPassInfo create(const Renderer &renderer,
                              const MeshLayout &layout,
                              SDL_GPUTexture *depth_texture = NULL);
  void release(SDL_GPUDevice *device);
};

inline void DepthPassInfo::release(SDL_GPUDevice *device) {
  if (depthTexture) {
    SDL_ReleaseGPUTexture(device, depthTexture);
    depthTexture = NULL;
  }
  if (pipeline) {
    SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
    pipeline = NULL;
  }
}

/// \brief Shadow pass configuration, to use in createShadowPass().
struct ShadowPassConfig {
  // default is 2k x 2k texture
  Uint32 width = 2048;
  Uint32 height = 2048;
};

/// \brief Run a shadow (depth-testing) pass using specific config.
/// \sa DepthPassInfo::create()
DepthPassInfo createShadowPass(const Renderer &renderer,
                               const MeshLayout &layout,
                               const ShadowPassConfig &config);

/// \ingroup depth_pass
/// \brief Struct for shadow-casting or opaque objects. For use in depth or
/// light pre-passes.
/// \sa renderDepthOnlyPass()
/// \sa renderShadowPass()
struct OpaqueCastable {
  // this could be the entire mesh
  MeshView mesh;
  GpuMat4 transform;
};

/// \ingroup depth_pass
/// \brief Render a depth-only pass, built from a set of OpaqueCastable.
void renderDepthOnlyPass(Renderer &renderer, const DepthPassInfo &passInfo,
                         const GpuMat4 &viewProj,
                         std::span<const OpaqueCastable> castables);

struct AABB;

/// \brief Render shadow pass, using provided scene bounds.
///
/// The scene bounds are in view-space.
void renderShadowPass(Renderer &renderer, DepthPassInfo &passInfo,
                      const DirectionalLight &dirLight,
                      std::span<const OpaqueCastable> castables,
                      const AABB &sceneBounds);

} // namespace candlewick
