#pragma once

#include "Camera.h"

namespace candlewick {

namespace camera_util {

  /// \name Camera control functions.
  ///
  /// These functions are meant for modifying a camera's pose and view.
  /// \{

  inline void localRotateX(Camera &camera, Radf angle) {
    float c, s;
    c = std::cos(angle);
    s = std::sin(angle);
    Mat3f R;
    R << 1., 0, 0, //
        0, c, -s,  //
        0, s, +c;
    camera.view.linear().applyOnTheLeft(R);
  }

  void rotateAroundPoint(Camera &camera, const Mat3f &R, const Float3 &p);

  /// \brief Rotate the camera around the center by a given increment.
  inline void rotateZAroundPoint(Camera &camera, Radf angle, const Float3 &p) {
    float c, s;
    c = std::cos(angle);
    s = std::sin(angle);
    Mat3f R;
    R << c, -s, 0., //
        s, +c, 0.,  //
        0., 0., 1.;
    rotateAroundPoint(camera, R, p);
  }

  inline void localTranslate(Camera &camera, const Float3 &tr) {
    camera.view.translation() += tr;
  }

  inline void localTranslateX(Camera &camera, float step) {
    camera.view(0, 3) += step;
  }

  inline void localTranslateZ(Camera &camera, float step) {
    camera.view(2, 3) += step;
  }

  inline void worldTranslate(Camera &camera, const Float3 &tr) {
    camera.view.translate(-tr);
  }

  inline void worldTranslateZ(Camera &camera, float step) {
    worldTranslate(camera, {0, 0, step});
  }

  inline void setWorldPosition(Camera &camera, const Float3 &pos) {
    auto Rt = camera.view.linear();
    camera.view.translation() = -Rt * pos;
  }

} // namespace camera_util

struct CylinderCameraControl {
  Camera &camera;
  Float3 target{0.f, 0.f, 0.f};

  static constexpr bool DEFAULT_Y_INVERT = false;

  CylinderCameraControl(Camera &_camera) : camera(_camera) {}

  auto &updateLookAt() {
    camera.view = lookAt(camera.position(), target);
    return *this;
  }

  auto &translate(const Float3 &tr) {
    target += tr;
    camera_util::worldTranslate(camera, tr);
    return *this;
  }

  auto &localTranslate(const Float3 &tr) {
    target += camera.transformVector(tr);
    camera_util::localTranslate(camera, tr);
    return *this;
  }

  auto &dolly(float height) { return translate({0, 0, height}); }

  auto &orbit(Radf angle) {
    camera_util::rotateZAroundPoint(camera, angle, target);
    return *this;
  }

  auto &viewportDrag(Float2 step, float rotSensitivity, float panSensitivity,
                     bool yinvert = DEFAULT_Y_INVERT) {
    const float rotStep = step.x() * rotSensitivity;
    const float panStep = step.y() * panSensitivity;
    float ystep = yinvert ? -panStep : panStep;
    return dolly(ystep).orbit(Rad{rotStep});
  }

  CylinderCameraControl &pan(Float2 step, float sensitivity);

  CylinderCameraControl &moveInOut(float scale, float offset);
};

/// \}

} // namespace candlewick
