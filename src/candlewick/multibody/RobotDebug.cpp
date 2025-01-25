#include "RobotDebug.h"
#include "../core/CameraControl.h"
#include "../primitives/Arrow.h"

#include <pinocchio/multibody/data.hpp>

namespace candlewick::multibody {

RobotDebugModule::RobotDebugModule(DebugScene &scene, const pin::Data &data)
    : DebugModule(scene), triad{NoInit}, arrow{NoInit}, pinData{data} {
  auto triad_data = createTriad();
  triad = createMeshFromBatch(scene.device(), triad_data, true);
  auto arrow_data = createArrow();
  arrow = createMesh(scene.device(), arrow_data);
  uploadMeshToDevice(scene.device(), arrow, arrow_data);

  for (Uint32 i = 0; i < 3; i++) {
    this->triadColors[i] = triad_data[i].material.baseColor;
  }
}

void RobotDebugModule::addDrawCommands(DebugScene &scene,
                                       const Camera &camera) {
  GpuMat4 viewProj = camera.viewProj();
  auto frame_tr = pinData.get().oMf[frameId].cast<float>();
  auto frame_pose = frame_tr.toHomogeneousMatrix();
  if (enableTriad) {
    scene.drawCommands.push_back({
        .pipeline_type = DebugPipelines::TRIANGLE_FILL,
        .mesh = &triad,
        .mvp{viewProj * frame_pose},
        .colors{triadColors.begin(), triadColors.end()},
    });
  }
}

} // namespace candlewick::multibody
