#include "MeshData.h"
#include "../core/Mesh.h"
#include "../core/MeshLayout.h"

namespace candlewick {

Mesh convertToMesh(const MeshData &meshData) {
  MeshLayout layout;
  Mesh mesh{layout};
  return mesh;
}

} // namespace candlewick
