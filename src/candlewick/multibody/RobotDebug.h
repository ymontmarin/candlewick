#pragma once

#include "Multibody.h"
#include "Components.h"
#include "../core/DebugScene.h"

namespace candlewick::multibody {

/// \brief A debug system for use with Pinocchio geometries.
///
/// Supports drawing a triad for a frame. Member \ref pinData must
/// refer to an existing pinocchio::Data object at all times.
struct RobotDebugSystem final : IDebugSubSystem {
  inline static const Float3 DEFAULT_TRIAD_SCALE = Float3::Constant(0.3333f);

  RobotDebugSystem(const pin::Model &model, const pin::Data &data)
      : IDebugSubSystem(), pinModel(model), pinData(data) {}

  entt::entity addFrameTriad(DebugScene &scene, pin::FrameIndex frame_id,
                             const Float3 &scale = DEFAULT_TRIAD_SCALE) {
    entt::registry &reg = scene.registry();
    auto [ent, triad] = scene.addTriad();
    reg.emplace<DebugScaleComponent>(ent, scale);
    reg.emplace<PinFrameComponent>(ent, frame_id);
    return ent;
  }

  entt::entity addFrameVelocityArrow(DebugScene &scene,
                                     pin::FrameIndex frame_id);

  /// \brief Update the visualization of the frame placements and their
  /// velocities.
  ///
  /// \warning We expect pinocchio::updateFramePlacements() or
  /// pinocchio::framesForwardKinematics() to be called first!
  void update(DebugScene &scene) {
    updateFrames(scene.registry());
    updateFrameVelocities(scene.registry());
  }

private:
  void updateFrames(entt::registry &reg);
  void updateFrameVelocities(entt::registry &reg);

  const pin::Model &pinModel;
  const pin::Data &pinData;
};

} // namespace candlewick::multibody
