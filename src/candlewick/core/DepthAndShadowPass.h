#pragma once

#include "Core.h"
#include "Mesh.h"
#include "math_types.h"
#include "LightUniforms.h"

#include <span>

namespace candlewick {

/// \brief Helper struct for depth or light pre-passes.
/// \warning The depth pre-pass is meant for _opaque_ geometries. Hence, the
/// pipeline created by the create() factory function will have back-face
/// culling enabled.
struct DepthPassInfo {
  enum DepthPassSlots : Uint32 {
    VIEWPROJ_SLOT = 0,
    TRANSFORM_SLOT = 1,
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

/// \sa DepthPassInfo::create()
DepthPassInfo createShadowPass(const Renderer &renderer,
                               const MeshLayout &layout, ShadowPassConfig);

/// \brief Struct for shadow-casting or opaque objects. For use in depth or
/// light pre-passes.
/// \sa renderDepthOnlyPass()
/// \sa renderShadowPass()
struct OpaqueCastable {
  // this could be the entire mesh
  MeshView mesh;
  GpuMat4 transform;
};

void renderDepthOnlyPass(Renderer &renderer, const DepthPassInfo &passInfo,
                         const GpuMat4 &viewProj,
                         std::span<const OpaqueCastable> castables);

struct AABB;

void renderShadowPass(Renderer &renderer, DepthPassInfo &shadowing,
                      const DirectionalLight &dirLight,
                      std::span<const OpaqueCastable> castables,
                      const AABB &sceneBounds);

} // namespace candlewick
