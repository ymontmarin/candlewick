#include "MeshData.h"
#include "../core/Mesh.h"
#include "../core/MeshLayout.h"

namespace candlewick {

Mesh convertToMesh(const MeshData &meshData) {
  MeshLayout layout;
  Mesh mesh{layout};
  return mesh;
}
MeshData::MeshData(std::vector<Vertex> vertexData,
                   std::vector<IndexType> indexData)
    : vertexData(std::move(vertexData)), indexData(std::move(indexData)) {}

} // namespace candlewick
