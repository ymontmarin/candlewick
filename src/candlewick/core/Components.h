#pragma once
#include "math_types.h"

namespace candlewick {

/// Tag struct for denoting an entity as opaque, for render pass organization.
struct Opaque {};

/// Tag struct for disabled (invisible) entities.
struct Disable {};

/// \brief A visibility component for ECS.
struct VisibilityComponent {
  bool status;
  operator bool() const { return status; }
};

// Tag environment entities
struct EnvironmentTag {};

struct TransformComponent : Mat4f {
  using Mat4f::Mat4f;
  using Mat4f::operator=;
};

}; // namespace candlewick
