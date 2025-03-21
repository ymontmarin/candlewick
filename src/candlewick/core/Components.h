#pragma once
#include "math_types.h"
#include "Mesh.h"
#include "MaterialUniform.h"

namespace candlewick {

/// Tag struct for denoting an entity as opaque, for render pass organization.
struct Opaque {};

/// Tag struct for disabled (invisible) entities.
struct Disable {};

// Tag environment entities
struct EnvironmentTag {};

struct TransformComponent : Mat4f {
  using Mat4f::Mat4f;
  using Mat4f::operator=;
};

struct MeshMaterialComponent {
  Mesh mesh;
  std::vector<PbrMaterial> materials;
  MeshMaterialComponent(Mesh &&mesh, std::vector<PbrMaterial> &&materials)
      : mesh(std::move(mesh)), materials(std::move(materials)) {
    assert(mesh.numViews() == materials.size());
  }
};

} // namespace candlewick
