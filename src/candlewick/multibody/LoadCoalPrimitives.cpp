#include "LoadCoalPrimitives.h"

#include "../core/errors.h"

#include "../primitives/Plane.h"
#include "../primitives/Cube.h"
#include "../primitives/Heightfield.h"
#include "../utils/MeshTransforms.h"
#include "../third-party/magic_enum.hpp"

#include <coal/shape/geometric_shapes.h>
#include <coal/hfield.h>
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_log.h>

namespace candlewick {

using Eigen::Matrix4f;

constexpr float kPlaneScale = 10.f;

void getPlaneOrHalfspaceNormalOffset(
    const hpp::fcl::CollisionGeometry &geometry, Float3 &n, float &d) {
  using namespace hpp::fcl;
  switch (geometry.getNodeType()) {
  case GEOM_PLANE: {
    const Plane &g = static_cast<const Plane &>(geometry);
    n = g.n.cast<float>();
    d = float(g.d);
    return;
  }
  case GEOM_HALFSPACE: {
    const Halfspace &g = static_cast<const Halfspace &>(geometry);
    n = g.n.cast<float>();
    d = float(g.d);
    return;
  }
  default:
    CDW_UNREACHABLE_ASSERT("This function should not be called with a "
                           "non-Plane, non-Halfspace coal CollisionGeometry.");
  }
}

MeshData loadCoalPrimitive(const hpp::fcl::CollisionGeometry &geometry,
                           const Float4 &meshColor, const Float3 &meshScale) {
  using namespace hpp::fcl;
  SDL_assert_always(geometry.getObjectType() == OT_GEOM);
  MeshData meshData{NoInit};
  Eigen::Affine3f transform = Eigen::Affine3f::Identity();
  const NODE_TYPE nodeType = geometry.getNodeType();
  SDL_Log("Loading Coal primitive of node type %s (%d)",
          magic_enum::enum_name(nodeType).data(), nodeType);
  switch (nodeType) {
  case GEOM_BOX: {
    const Box &g = static_cast<const Box &>(geometry);
    transform.scale(2 * g.halfSide.cast<float>());
    meshData = toOwningMeshData(loadCube());
    break;
  }
  case GEOM_HALFSPACE:
  case GEOM_PLANE: {
    meshData = toOwningMeshData(loadPlane());
    Float3 n;
    float d;
    getPlaneOrHalfspaceNormalOffset(geometry, n, d);
    const auto quat = Eigen::Quaternionf::FromTwoVectors(Float3::UnitZ(), n);
    transform.scale(kPlaneScale).rotate(quat).translate(d * Float3::UnitZ());
    break;
  }
  default:
    throw std::runtime_error("Unsupported geometry type.");
    break;
  }
  meshData.material.baseColor = meshColor;
  transform.scale(meshScale);
  apply3DTransformInPlace(meshData, transform);
  return meshData;
}

namespace detail {
template <typename BV>
MeshData load_coal_heightfield_impl(const coal::HeightField<BV> &hf) {
  Eigen::MatrixXf heights = hf.getHeights().template cast<float>();
  Eigen::VectorXf xgrid = hf.getXGrid().template cast<float>();
  Eigen::VectorXf ygrid = hf.getYGrid().template cast<float>();

  return loadHeightfield(heights, xgrid, ygrid);
}

} // namespace detail

MeshData loadCoalHeightField(const coal::CollisionGeometry &collGeom) {
  SDL_assert(collGeom.getObjectType() == coal::OT_HFIELD);
  const coal::NODE_TYPE nodeType = collGeom.getNodeType();
  switch (nodeType) {
  case coal::HF_AABB:
  case coal::BV_AABB: {
    auto &geom = static_cast<const coal::HeightField<coal::AABB> &>(collGeom);
    return detail::load_coal_heightfield_impl(geom);
  }
  case coal::HF_OBBRSS:
  case coal::BV_OBBRSS: {
    auto &geom = static_cast<const coal::HeightField<coal::OBBRSS> &>(collGeom);
    return detail::load_coal_heightfield_impl(geom);
  }
  default:
    throw std::runtime_error("Unsupported nodeType.");
  }
}
} // namespace candlewick
