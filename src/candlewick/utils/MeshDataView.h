#pragma once

#include <span>
#include "MeshData.h"

namespace candlewick {
struct MeshDataView : MeshDataBase<MeshDataView> {
  using Vertex = MeshData::Vertex;
  using IndexType = MeshData::IndexType;
  SDL_GPUPrimitiveType primitiveType;
  std::span<const Vertex> vertexData;
  std::span<const IndexType> indexData;

  MeshDataView(const MeshData &meshData);
  MeshDataView(const MeshData &meshData, size_t offset, size_t len);

  MeshDataView(SDL_GPUPrimitiveType primitiveType,
               std::span<const Vertex> vertices,
               std::span<const IndexType> indices = {});
};

MeshData toOwningMeshData(const MeshDataView &view);

} // namespace candlewick
