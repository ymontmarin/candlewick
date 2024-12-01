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

struct alignas(16) DefaultVertex {
  GpuVec3 pos;
  alignas(16) GpuVec3 normal;
  alignas(16) GpuVec4 color;
};

template <> struct VertexTraits<DefaultVertex> {
  static constexpr auto layout() {
    return MeshLayout{}
        .addBinding(0, sizeof(DefaultVertex))
        .addAttribute(0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                      offsetof(DefaultVertex, pos))
        .addAttribute(1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                      offsetof(DefaultVertex, normal))
        .addAttribute(2, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
                      offsetof(DefaultVertex, color));
  }
};

struct MeshData : MeshDataBase<MeshData> {
  using IndexType = Uint32;
  SDL_GPUPrimitiveType primitiveType;    //< Geometry primitive for the mesh
  std::vector<DefaultVertex> vertexData; //< Vertices
  std::vector<IndexType> indexData;      //< Indices for indexed mesh. Optional.
  PbrMaterialData material;

  explicit MeshData() = default;
  MeshData(const MeshData &) = delete;
  MeshData(MeshData &&) noexcept = default;
  MeshData &operator=(MeshData &&) noexcept = default;
  MeshData(SDL_GPUPrimitiveType primitiveType,
           std::vector<DefaultVertex> vertexData,
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
