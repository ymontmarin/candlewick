#pragma once

#include "../core/Core.h"
#include "../core/Tags.h"
#include "../core/MeshLayout.h"
#include "VertexDataBlob.h"
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
  using IndexType = Uint32;
  SDL_GPUPrimitiveType primitiveType; //< Geometry primitive for the mesh
  VertexDataBlob vertexData;          //< Vertices
  std::vector<IndexType> indexData;   //< Indices for indexed mesh. Optional.
  PbrMaterialData material;

  const MeshLayout &layout() const { return vertexData.layout(); }

  explicit MeshData(NoInitT);
  template <IsVertexType VertexT>
  explicit MeshData(SDL_GPUPrimitiveType primitiveType,
                    std::vector<VertexT> vertexData,
                    std::vector<IndexType> indexData);
  explicit MeshData(SDL_GPUPrimitiveType primitiveType, MeshLayout layout,
                    std::vector<char> vertexData,
                    std::vector<IndexType> indexData);
  MeshData(const MeshData &) = delete;
  MeshData(MeshData &&) noexcept = default;
  MeshData &operator=(MeshData &&) noexcept = default;
};

template <IsVertexType VertexT>
MeshData::MeshData(SDL_GPUPrimitiveType primitiveType,
                   std::vector<VertexT> vertexData,
                   std::vector<IndexType> indexData)
    : primitiveType(primitiveType), vertexData(std::move(vertexData)),
      indexData(std::move(indexData)) {}

/// \brief Convert @c MeshData to a @c Mesh. This creates the required
/// vertex buffer and index buffer (if required).
Mesh convertToMesh(const Device &device, const MeshData &meshData);

/// \p vertexOffset Vertex buffer offset (in bytes)
/// \p indexOffset Index buffer offset (in bytes)
Mesh convertToMesh(const MeshData &meshData, SDL_GPUBuffer *vertexBuffer,
                   Uint64 vertexOffset, SDL_GPUBuffer *indexBuffer,
                   Uint64 indexOffset, bool takeOwnership = false);

/// \brief Upload the contents of a single, individual mesh to the GPU device.
void uploadMeshToDevice(const Device &device, const Mesh &mesh,
                        const MeshData &meshData);
} // namespace candlewick
