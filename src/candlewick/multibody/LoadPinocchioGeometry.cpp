#include "LoadPinocchioGeometry.h"
#include "LoadCoalPrimitives.h"
#include "../core/errors.h"
#include "../utils/LoadMesh.h"
#include "../third-party/magic_enum.hpp"

#include <pinocchio/multibody/geometry.hpp>

namespace candlewick::multibody {
using hpp::fcl::CollisionGeometry;

void loadGeometryObject(const pin::GeometryObject &gobj,
                        std::vector<MeshData> &meshData) {
  using hpp::fcl::OBJECT_TYPE;
  using enum OBJECT_TYPE;
  using hpp::fcl::NODE_TYPE;

  const CollisionGeometry &collgom = *gobj.geometry.get();
  const OBJECT_TYPE objType = collgom.getObjectType();

  Float4 meshColor = gobj.meshColor.cast<float>();
  Float3 meshScale = gobj.meshScale.cast<float>();

  switch (objType) {
  case OT_BVH: {
    loadSceneMeshes(gobj.meshPath.c_str(), meshData);
    break;
  }
  case OT_GEOM: {
    meshData.push_back(loadCoalPrimitive(collgom, meshColor, meshScale));
    break;
  }
  case OT_HFIELD: {
    MeshData &md = meshData.emplace_back(loadCoalHeightField(collgom));
    md.material.baseColor = meshColor;
    break;
  }
  default:
    throw InvalidArgument("Unsupported object type.");
    break;
  }
}

} // namespace candlewick::multibody
