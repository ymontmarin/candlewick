#include "LoadPinocchioGeometry.h"
#include "LoadCoalPrimitives.h"
#include "../utils/LoadMesh.h"

#include <pinocchio/multibody/geometry.hpp>
#include <pinocchio/multibody/geometry-object.hpp>

namespace candlewick::multibody {
using hpp::fcl::CollisionGeometry;

constexpr const char *obj_type_str(hpp::fcl::OBJECT_TYPE type) {
  using enum hpp::fcl::OBJECT_TYPE;
  switch (type) {
  case OT_UNKNOWN:
    return "unknown";
  case OT_BVH:
    return "bvh";
  case OT_GEOM:
    return "geom";
  case OT_OCTREE:
    return "octree";
  case OT_HFIELD:
    return "hfield";
  case OT_COUNT:
    return "count";
  }
}

void loadGeometryObject(const pin::GeometryObject &gobj,
                        std::vector<MeshData> &meshData) {
  using hpp::fcl::OBJECT_TYPE;
  using enum OBJECT_TYPE;
  using hpp::fcl::NODE_TYPE;

  const CollisionGeometry &collgom = *gobj.geometry.get();
  const OBJECT_TYPE objType = collgom.getObjectType();
  printf("Processing GeometryObject %s.\t", gobj.name.c_str());
  printf("Got object type %s\n", obj_type_str(objType));

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
    throw std::runtime_error("Unsupported object type.");
    break;
  }
}

void loadGeometryModel(const pin::GeometryModel &model,
                       std::vector<MeshData> &meshData) {
  const size_t ngeoms = model.ngeoms;

  for (size_t i = 0; i < ngeoms; i++) {
    const pin::GeometryObject &gobj = model.geometryObjects[i];
    loadGeometryObject(gobj, meshData);
  }
}

} // namespace candlewick::multibody
