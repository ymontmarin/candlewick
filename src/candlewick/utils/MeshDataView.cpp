#include "MeshDataView.h"

namespace candlewick {

MeshDataView::MeshDataView(const MeshData &meshData)
    : primitiveType(meshData.primitiveType), vertexData(meshData.vertexData),
      indexData(meshData.indexData) {}

MeshDataView::MeshDataView(const MeshData &meshData, size_t offset, size_t len)
    : primitiveType(meshData.primitiveType),
      vertexData(meshData.vertexData.begin() + offset, len),
      indexData(meshData.indexData.begin() + offset, len) {}

MeshData toOwningMeshData(const MeshDataView &view) {
  std::vector<MeshData::Vertex> vertexData;
  vertexData.assign(view.vertexData.begin(), view.vertexData.end());
  std::vector<MeshData::IndexType> indexData;
  indexData.assign(view.indexData.begin(), view.indexData.end());
  return MeshData{view.primitiveType, std::move(vertexData),
                  std::move(indexData)};
}

} // namespace candlewick
