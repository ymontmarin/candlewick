#pragma once

#include <pinocchio/multibody/fwd.hpp>

namespace candlewick::multibody {
namespace pin = pinocchio;

struct PinGeomObjComponent {
  pin::GeomIndex geom_index;
  operator auto() const { return geom_index; }
};

struct PinFrameComponent {
  pin::FrameIndex frame_id;
  operator auto() const { return frame_id; }
};

/// Similar to PinFrameComponent, but tags the entity to render the frame
/// velocity - not its placement as e.g. a triad.
struct PinFrameVelocityComponent {
  pin::FrameIndex frame_id;
  operator auto() const { return frame_id; }
};

} // namespace candlewick::multibody
