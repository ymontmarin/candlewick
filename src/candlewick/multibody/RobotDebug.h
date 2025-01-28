#pragma once

#include "Multibody.h"
#include "../core/DebugScene.h"

namespace candlewick::multibody {

/// \brief A debug module for use with Pinocchio geometries.
/// \details Supports drawing a triad for a frame. Member \ref pinData must
/// refer to an existing pinocchio::Data object at all times.
struct RobotDebugModule final : DebugModule {
  Mesh triad;
  std::array<GpuVec4, 3> triadColors;
  Mesh arrow;
  std::reference_wrapper<const pin::Data> pinData;

  pin::FrameIndex frameId;
  bool enableTriad{true};
  bool enableArrow{true};

  RobotDebugModule(DebugScene &scene, const pin::Data &data);
  virtual void addDrawCommands(DebugScene &scene,
                               const Camera &camera) override;
};

} // namespace candlewick::multibody
