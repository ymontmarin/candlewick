#include "MeshData.h"
#include "MeshTransforms.h"

namespace candlewick {

void applyTransformToMeshInPlace(MeshData &meshData,
                                 const Eigen::Affine3f &tr) {
  Eigen::Matrix3f normalMatrix = tr.linear().inverse().transpose();

  for (MeshData::Vertex &v : meshData.vertexData) {
    v.pos = tr * v.pos;
    v.normal = normalMatrix * v.normal;
  }
}

} // namespace candlewick
