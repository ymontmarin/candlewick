#pragma once

#include "../core/Core.h"
#include "../core/math_types.h"
#include "MaterialData.h"
#include <SDL3/SDL_gpu.h>

namespace candlewick {

struct Mesh;

template <typename Derived> struct MeshDataBase {
  Derived &derived() { return static_cast<Derived &>(*this); }
  const Derived &derived() const { return static_cast<const Derived &>(*this); }

  std::size_t numVertices() const { return derived().vertexData.size(); }
  std::size_t numIndices() const { return derived().indexData.size(); }
  bool isIndexed() const { return numIndices() > 0; }
};

struct MeshData : MeshDataBase<MeshData> {
  // use a vertex struct, allows us to interleave data properly
  struct alignas(16) Vertex {
    GpuVec3 pos;
    alignas(16) GpuVec3 normal;
  };
  using IndexType = Uint32;
  SDL_GPUPrimitiveType primitiveType;
  std::vector<Vertex> vertexData;
  std::vector<IndexType> indexData;
  PbrMaterialData material;

  explicit MeshData() = default;
  MeshData(const MeshData &) = delete;
  MeshData(MeshData &&) noexcept = default;
  MeshData &operator=(MeshData &&) noexcept = default;
  MeshData(SDL_GPUPrimitiveType primitiveType, std::vector<Vertex> vertexData,
           std::vector<IndexType> indexData);
};

/// \brief Convert @c MeshData to a @c Mesh. This creates the required
/// vertex buffer and index buffer (if required).
Mesh convertToMesh(const Device &device, const MeshData &meshData);

Mesh convertToMesh(const MeshData &meshData, SDL_GPUBuffer *vertexBuffer,
                   Uint64 vertexOffset, SDL_GPUBuffer *indexBuffer,
                   Uint64 indexOffset, bool takeOwnership = false);

/// \brief Upload mesh contents to GPU device.
void uploadMeshToDevice(const Device &device, const Mesh &mesh,
                        const MeshData &meshData);
} // namespace candlewick
