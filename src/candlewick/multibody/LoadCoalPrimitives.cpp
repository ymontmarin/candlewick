#include "LoadCoalPrimitives.h"

#include "../primitives/Plane.h"
#include "../primitives/Cube.h"
#include "../utils/MeshTransforms.h"

#include <hpp/fcl/collision_object.h>
#include <SDL3/SDL_assert.h>

namespace candlewick {

using Eigen::Matrix4f;

MeshData loadCoalPrimitive(const hpp::fcl::CollisionGeometry &geometry,
                           const Float4 &meshColor, const Float3 &meshScale) {
  using hpp::fcl::NODE_TYPE;
  SDL_assert_always(geometry.getObjectType() == hpp::fcl::OT_GEOM);
  MeshData meshData;
  switch (geometry.getNodeType()) {
  case NODE_TYPE::GEOM_BOX: {
    meshData = toOwningMeshData(loadCube());
    break;
  }
  case NODE_TYPE::GEOM_PLANE: {
    meshData = toOwningMeshData(loadPlane());
    break;
  }
  default:
    throw std::runtime_error("Unsupported geometry type.");
    break;
  }
  meshData.material.baseColor = meshColor;
  Eigen::Affine3f tr = Eigen::Affine3f::Identity();
  tr.scale(meshScale);
  apply3DTransformInPlace(meshData, tr);
  return meshData;
}

} // namespace candlewick
