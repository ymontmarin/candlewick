#include "LoadPinocchioGeometry.h"
#include "LoadCoalPrimitives.h"
#include "../core/errors.h"
#include "../utils/LoadMesh.h"
#include "../utils/MeshTransforms.h"

#include <pinocchio/multibody/geometry.hpp>

namespace candlewick::multibody {

void loadGeometryObject(const pin::GeometryObject &gobj,
                        std::vector<MeshData> &meshData) {
  using namespace coal;

  const coal::CollisionGeometry &collgom = *gobj.geometry.get();
  const OBJECT_TYPE objType = collgom.getObjectType();

  Float4 meshColor = gobj.meshColor.cast<float>();
  Float3 meshScale = gobj.meshScale.cast<float>();

  switch (objType) {
  case OT_BVH: {
    loadSceneMeshes(gobj.meshPath.c_str(), meshData);
    Eigen::Affine3f T;
    T.setIdentity();
    T.scale(meshScale);
    for (auto &d : meshData) {
      apply3DTransformInPlace(d, T);
    }
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
