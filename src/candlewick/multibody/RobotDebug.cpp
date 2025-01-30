#include "RobotDebug.h"

#include "../primitives/Arrow.h"

#include <pinocchio/algorithm/frames.hpp>

namespace candlewick::multibody {

entt::entity RobotDebugSystem::addFrameVelocityArrow(DebugScene &scene,
                                                     pin::FrameIndex frame_id) {
  entt::registry &reg = scene.registry();
  MeshData arrow_data = createArrow(false);
  Mesh mesh = createMesh(scene.device(), arrow_data, true);
  GpuVec4 color = 0xFF217Eff_rgbaf;

  auto entity = reg.create();
  reg.emplace<DebugMeshComponent>(entity, DebugPipelines::TRIANGLE_FILL,
                                  std::move(mesh), std::vector{color},
                                  Mat4f::Identity());
  reg.emplace<PinFrameVelocityComponent>(entity, frame_id);
  return entity;
}

void RobotDebugSystem::updateFrames(entt::registry &reg) {
  auto view = reg.view<const PinFrameComponent, const DebugScaleComponent,
                       DebugMeshComponent>();
  for (auto &&[ent, frame_id, scale, dmc] : view.each()) {
    Mat4f pose{pinData.oMf[frame_id].cast<float>()};
    auto D = scale.value.asDiagonal();
    pose.topLeftCorner<3, 3>().applyOnTheRight(D);
    dmc.transform = pose;
  }
}

void RobotDebugSystem::updateFrameVelocities(entt::registry &reg) {
  using Motionf = pin::MotionTpl<float>;
  using SE3f = pin::SE3Tpl<float>;
  constexpr float vel_scale = 0.5f;

  auto view = reg.view<const PinFrameVelocityComponent, DebugMeshComponent>();
  for (auto &&[ent, fvc, dmc] : view.each()) {
    Motionf vel =
        pin::getFrameVelocity(pinModel, pinData, fvc, pin::LOCAL).cast<float>();

    const SE3f pose = pinData.oMf[fvc].cast<float>();
    Eigen::Quaternionf quatf;
    Eigen::DiagonalMatrix<float, 3> scaleMatrix;
    dmc.transform = pose.toHomogeneousMatrix();
    auto v = vel.linear();
    scaleMatrix = Eigen::Scaling(0.3f, 0.3f, vel_scale * v.norm());

    // the arrow mesh is posed z-up by default.
    // we need to rotate towards where the velocity is pointing,
    // then transform to the frame space.
    quatf.setFromTwoVectors(Float3::UnitZ(), -v);
    Mat3f R2 = quatf.toRotationMatrix() * scaleMatrix;
    auto R = dmc.transform.topLeftCorner<3, 3>();
    R.applyOnTheRight(R2);
  }
}

} // namespace candlewick::multibody
