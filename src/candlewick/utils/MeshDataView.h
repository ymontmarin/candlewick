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
  template <size_t N, size_t M>
  MeshDataView(SDL_GPUPrimitiveType primitiveType, const Vertex (&vertices)[N],
               const IndexType (&indices)[M]);

  template <size_t N>
  MeshDataView(SDL_GPUPrimitiveType primitiveType, const Vertex (&vertices)[N]);
};

template <size_t N, size_t M>
MeshDataView::MeshDataView(SDL_GPUPrimitiveType primitiveType,
                           const Vertex (&vertices)[N],
                           const IndexType (&indices)[M])
    : primitiveType(primitiveType), vertexData(vertices), indexData(indices) {}

template <size_t N>
MeshDataView::MeshDataView(SDL_GPUPrimitiveType primitiveType,
                           const Vertex (&vertices)[N])
    : primitiveType(primitiveType), vertexData(vertices), indexData() {}

MeshData toOwningMeshData(MeshDataView view);

} // namespace candlewick
