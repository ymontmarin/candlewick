#pragma once

#include "Camera.h"
#include <SDL3/SDL_keycode.h>

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

  /// \brief Rotate the camera around the center by a given increment.
  inline void worldRotateZ(Camera &camera, Radf angle) {
    float c, s;
    c = std::cos(angle);
    s = std::sin(angle);
    Mat3f R;
    R << c, -s, 0., //
        s, +c, 0.,  //
        0., 0., 1.;
    camera.view.linear().applyOnTheRight(R);
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
    auto R = camera.view.linear();
    auto p = camera.view.translation();
    p += R * tr;
  }

  inline void worldTranslateZ(Camera &camera, float step) {
    worldTranslate(camera, {0, 0, step});
  }

  inline void setWorldPosition(Camera &camera, const Float3 &pos) {
    auto R = camera.view.linear();
    camera.view.translation() = R * pos;
  }

} // namespace camera_util

template <class U>
concept CameraController = requires(U u) {
  { u.camera } -> std::convertible_to<Camera &>;
  { u.dolly(std::declval<float>()) } -> std::same_as<U &>;
};

struct CylinderCameraControl {
  Camera &camera;

  static constexpr bool DEFAULT_Y_INVERT = true;

  auto &dolly(float height) {
    camera_util::worldTranslateZ(camera, height);
    return *this;
  }

  auto &orbit(Radf angle) {
    camera_util::worldRotateZ(camera, angle);
    return *this;
  }

  auto &viewportDrag(Float2 step, float rotSensitivity, float panSensitivity,
                     bool yinvert = DEFAULT_Y_INVERT) {
    const float rotStep = step.x() * rotSensitivity;
    const float panStep = step.y() * panSensitivity;
    float ystep = yinvert ? -panStep : panStep;
    return dolly(ystep).orbit(Rad{rotStep});
  }

  auto &moveInOut(float scale, float offset) {
    const float alpha = 1.f - (offset > 0 ? 1.f / scale : scale);
    const float curDist = camera.view.translation().norm();
    camera_util::localTranslateZ(camera, curDist * alpha);
    return *this;
  }
};

static_assert(CameraController<CylinderCameraControl>);

/// \}

} // namespace candlewick
