#pragma once

#include "Device.h"
#include "Mesh.h"

#include <span>
#include <SDL3/SDL_gpu.h>

namespace candlewick {

/// \brief The Renderer class provides a rendering context for a graphical
/// application.
///
/// \sa Scene
/// \sa Device
/// \sa Mesh
struct Renderer {
  Device device;
  SDL_Window *window = nullptr;
  SDL_GPUTexture *swapchain;
  SDL_GPUTexture *depth_texture = nullptr;
  SDL_GPUTextureFormat depth_format;
  SDL_GPUCommandBuffer *command_buffer;

  Renderer(Device &&device, SDL_Window *window);
  Renderer(Device &&device, SDL_Window *window,
           SDL_GPUTextureFormat suggested_depth_format);

  /// Acquire the command buffer, starting a frame.
  void beginFrame() { command_buffer = SDL_AcquireGPUCommandBuffer(device); }

  /// Submit the command buffer, ending the frame.
  void endFrame() { SDL_SubmitGPUCommandBuffer(command_buffer); }

  bool waitAndAcquireSwapchain() {
    return SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, window,
                                                 &swapchain, NULL, NULL);
  }

  bool acquireSwapchain() {
    return SDL_AcquireGPUSwapchainTexture(command_buffer, window, &swapchain,
                                          NULL, NULL);
  }

  SDL_GPUTextureFormat getSwapchainTextureFormat() const {
    return SDL_GetGPUSwapchainTextureFormat(device, window);
  }

  bool hasDepthTexture() const { return depth_texture != nullptr; }

  /// Bind a MeshView object.
  void bindMeshView(SDL_GPURenderPass *pass, const MeshView &view);
  /// Bind the Mesh object.
  void bindMesh(SDL_GPURenderPass *pass, const Mesh &mesh) {
    bindMeshView(pass, mesh.toView());
  }

  /// Render an individual Mesh as part of a render pass, using a provided first
  /// index or vertex offset.
  /// \details This routine will bind the vertex and index buffer. Prefer one of
  /// the other routines to e.g. batch draw calls that use the same buffer
  /// bindings.
  /// \warning Call bindMesh() first!
  /// \param pass The GPU render pass
  /// \param mesh The Mesh to draw.
  void draw(SDL_GPURenderPass *pass, const Mesh &mesh) {
    this->drawView(pass, mesh.toView());
  }

  /// \brief Draw a MeshView.
  /// \warning Call bindMesh() first!
  /// \param pass The GPU render pass.
  /// \param mesh MeshView object to be drawn.
  /// \sa draw()
  void drawView(SDL_GPURenderPass *pass, const MeshView &mesh);

  /// \brief Render a collection of MeshView.
  /// This collection must satisfy the invariant that they all have the same
  /// parent Mesh object, which allows batching draw calls like this without
  /// having to rebind the Mesh.
  /// \param pass The GPU render pass
  /// \param meshViews Collection of MeshView objects with the same parent Mesh.
  /// \sa drawView() (for individual views)
  void drawViews(SDL_GPURenderPass *pass, std::span<const MeshView> meshViews);

  /// Render a collection of Mesh collected in a Shape object.
  /// The Shape class satisfies the necessary invariants to allow for
  /// batching. We only have to bind the vertex and index buffers once, here.
  /// \param pass The GPU render pass
  /// \param shape The Shape object
  // void render(SDL_GPURenderPass *pass, const Shape &shape);

  /// \brief Push uniform data to the vertex shader.
  /// \warning Call this after beginFrame() but **before** endFrame().
  void pushVertexUniform(Uint32 slot_index, const void *data, Uint32 length) {
    SDL_PushGPUVertexUniformData(command_buffer, slot_index, data, length);
  }
  /// \brief Push uniform data to the fragment shader.
  /// \warning Call this after beginFrame() but **before** endFrame().
  void pushFragmentUniform(Uint32 slot_index, const void *data, Uint32 length) {
    SDL_PushGPUFragmentUniformData(command_buffer, slot_index, data, length);
  }

  /// \brief Bind fragment sampler.
  void bindFragmentSampler(SDL_GPURenderPass *pass, Uint32 first_slot,
                           SDL_GPUTextureSamplerBinding binding);
  /// \brief Bind fragment samplers.
  void bindFragmentSamplers(
      SDL_GPURenderPass *pass, Uint32 first_slot,
      const std::vector<SDL_GPUTextureSamplerBinding> bindings);

  void destroy() {
    SDL_ReleaseWindowFromGPUDevice(device, window);
    device.destroy();
  }
};

} // namespace candlewick
