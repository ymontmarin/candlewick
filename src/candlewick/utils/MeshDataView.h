#pragma once

#include <span>
#include "MeshData.h"

namespace candlewick {
struct MeshDataView {
  using Vertex = MeshData::Vertex;
  using IndexType = MeshData::IndexType;
  std::span<const Vertex> vertexData;
  std::span<const IndexType> indexData;

  MeshDataView(const MeshData &meshData);
  MeshDataView(const MeshData &meshData, size_t offset, size_t len);
  template <size_t N, size_t M>
  MeshDataView(const Vertex (&vertices)[N], const IndexType (&indices)[M]);
};

template <size_t N, size_t M>
MeshDataView::MeshDataView(const Vertex (&vertices)[N],
                           const IndexType (&indices)[M])
    : vertexData(vertices), indexData(indices) {}

MeshData toOwningMeshData(MeshDataView view);

} // namespace candlewick
