#include "Visualizer.h"

#include <imgui.h>

namespace candlewick::multibody {

void default_gui_exec(Visualizer &viz, Renderer &render) {
  ImGui::Begin("Prout");
  ImGui::End();
}

void Visualizer::displayImpl() {

  DirectionalLightUniform myLight{
      .direction = {0., -1., -1.},
      .color = {1.0, 1.0, 1.0},
      .intensity = 8.0,
  };
  Matrix4f projectionMat;
  Matrix4f viewMat;
  viewMat.setIdentity();
  light_ubo_t lightUbo{myLight, cameraViewPos(viewMat)};
  auto &oMg = visualData.oMg;
  robotScene
      .setVertexUniform(
          Shape::TRANSFORM_SLOT,
          [&](pin::GeomIndex i) {
            const pin::SE3 &placement = oMg[i];
            Matrix4f modelMat = placement.cast<float>().toHomogeneousMatrix();
            Matrix3f normalMatrix =
                modelMat.topLeftCorner<3, 3>().inverse().transpose();
            Matrix4f modelView = viewMat * modelMat;
            Matrix4f mvp = projectionMat * modelView;
            return TransformUniformData{
                modelMat,
                mvp,
                normalMatrix,
            };
          })
      .setFragmentUniform(1, [&](auto) { return lightUbo; });
  robotScene.render(renderer);
}
} // namespace candlewick::multibody
