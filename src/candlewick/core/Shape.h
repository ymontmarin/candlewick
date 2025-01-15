#pragma once

#include "Mesh.h"
#include "../utils/MeshData.h"
#include "../utils/MaterialData.h"

#include <span>

namespace candlewick {

/// \brief Class containing the mesh \c Mesh handles for a set of meshes with
/// materials.
/// \invariant All the meshes share a given master vertex buffer and master
/// index buffer, to allow batching of draw calls.
class Shape {
private:
  Device const *device;
  SDL_GPUBuffer *masterVertexBuffer;
  SDL_GPUBuffer *masterIndexBuffer;

  std::vector<Mesh> meshes_;
  std::vector<PbrMaterialData> materials_;
  MeshLayout layout_;

  Shape(const Device &dev, SDL_GPUBuffer *vb, SDL_GPUBuffer *ib, MeshLayout l,
        std::vector<Mesh> &&m, std::vector<PbrMaterialData> &&mat)
      : device(&dev), masterVertexBuffer(vb), masterIndexBuffer(ib),
        meshes_(std::move(m)), materials_(std::move(mat)), layout_(l) {}

public:
  enum { MATERIAL_SLOT = 0 };

  Shape(const Shape &) = delete;
  Shape(Shape &&);

  SDL_GPUBufferBinding getVertexBinding() const {
    return {.buffer = masterVertexBuffer, .offset = 0};
  }
  SDL_GPUBufferBinding getIndexBinding() const {
    return {.buffer = masterIndexBuffer, .offset = 0};
  }
  const std::vector<Mesh> &meshes() const { return meshes_; }
  const std::vector<PbrMaterialData> &materials() const { return materials_; }
  const MeshLayout &layout() const { return layout_; }
  bool hasMaterials() const { return !materials_.empty(); }

  void release();
  ~Shape() { release(); }

  /// \brief Create a shape from a set of \c MeshData objects, optionally upload
  /// to GPU device.
  static Shape createShapeFromDatas(const Device &device,
                                    std::span<const MeshData> meshDatas,
                                    bool upload = false);

  /// Create a \c Shape object from a rvalue vector of \c MeshData objects.
  /// This will upload the data to the GPU straightaway.
  /// \overload createShapeFromDatas()
  static Shape createShapeFromDatas(const Device &device,
                                    std::vector<MeshData> &&meshDatas);

  template <std::size_t Size>
  static Shape createShapeFromDatas(const Device &device,
                                    std::array<MeshData, Size> &&meshDatas) {
    std::span<const MeshData> view{meshDatas};
    return createShapeFromDatas(device, view, true);
  }
};

} // namespace candlewick
