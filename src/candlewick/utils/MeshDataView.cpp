#include "MeshDataView.h"

namespace candlewick {

MeshDataView::MeshDataView(const MeshData &meshData)
    : vertexData(meshData.vertexData), indexData(meshData.indexData) {}

MeshDataView::MeshDataView(const MeshData &meshData, size_t offset, size_t len)
    : vertexData(meshData.vertexData.begin() + offset, len),
      indexData(meshData.indexData.begin() + offset, len) {}

MeshData toOwningMeshData(MeshDataView view) {
  std::vector<MeshData::Vertex> vertexData;
  vertexData.assign(view.vertexData.begin(), view.vertexData.end());
  std::vector<MeshData::IndexType> indexData;
  indexData.assign(view.indexData.begin(), view.indexData.end());
  return MeshData{std::move(vertexData), std::move(indexData)};
}

} // namespace candlewick
