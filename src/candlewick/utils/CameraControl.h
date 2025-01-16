#pragma once

#include "../core/math_types.h"

#include <Eigen/Geometry>

namespace candlewick {

/// \brief The main way of using a camera to render things.
struct Camera {
  Mat4f projection;
  Eigen::Isometry3f view;

  Eigen::Isometry3f pose() const { return view.inverse(); }
  Float3 position() const { return pose().translation(); }

  /// View-projection matrix \f$P * V\f$
  auto viewProj() const { return projection * view.matrix(); }

  /// \name Shortcuts for the basis vectors of the pose matrix, i.e. the
  /// world-space camera orientation vectors.
  /// \{

  Float3 right() const { return view.linear().row(0); }
  auto right() { return view.linear().row(0); }
  Float3 up() const { return view.linear().row(1); }
  auto up() { return view.linear().row(1); }
  Float3 forward() const { return -view.linear().row(2); }

  /// \}
};

/// \name Camera control functions.
/// These functions are prefixed `camera` but take the view matrix and
/// not the camera's pose matrix.
/// \{

inline void cameraLocalRotateX(Camera &camera, Rad<float> angle) {
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
inline void cameraWorldRotateZ(Camera &camera, Rad<float> angle) {
  float c, s;
  c = std::cos(angle);
  s = std::sin(angle);
  Mat3f R;
  R << c, -s, 0., //
      s, +c, 0.,  //
      0., 0., 1.;
  camera.view.linear().applyOnTheRight(R);
}

inline void cameraLocalTranslate(Camera &camera, const Float3 &tr) {
  camera.view.translation() += tr;
}

inline void cameraLocalTranslateX(Camera &camera, float step) {
  cameraLocalTranslate(camera, {step, 0, 0});
}

inline void cameraWorldTranslate(Camera &camera, const Float3 &tr) {
  auto R = camera.view.linear();
  auto p = camera.view.translation();
  p += R * tr;
}

inline void cameraWorldTranslateZ(Camera &camera, float step) {
  cameraWorldTranslate(camera, {0, 0, step});
}

/// Compute view matrix looking at \p center from \p eye, with
/// the camera pointing up towards \p up.
Mat4f lookAt(const Float3 &eye, const Float3 &center,
             const Float3 &up = Float3::UnitZ());

/// \brief Compute orthographic projection matrix, from clipping plane
/// parameters (left, right, top, bottom).
Mat4f perspectiveMatrix(float left, float right, float top, float bottom,
                        float near, float far);

/// \brief Get perspective projection matrix given fov, aspect ratio, and
/// clipping planes.
/// \p fovY Vertical field of view in radians
/// \p aspectRatio Width / Height
/// \p nearZ Near clipping plane
/// \p farZ Far clipping plane
Mat4f perspectiveFromFov(Radf fovY, float aspectRatio, float nearZ, float farZ);

Mat4f orthographicMatrix(const Float2 &size, float, float far);

inline void orthographicZoom(Mat4f &projMatrix, float factor) {
  projMatrix(0, 0) *= 1.f / factor;
  projMatrix(1, 1) *= 1.f / factor;
}

/// \}

struct CylinderCameraControl {
  Camera &camera;

  auto &viewportDrag(Float2 step, float rotSensitivity, float panSensitivity,
                     bool yinvert = true) {
    step.x() *= rotSensitivity;
    step.y() *= panSensitivity;
    float ystep = yinvert ? -step.y() : step.y();
    cameraWorldTranslateZ(camera, ystep);
    cameraWorldRotateZ(camera, Rad(step.x()));
    return *this;
  }

  auto &moveInOut(float scale, float offset) {
    const float alpha = 1.f - (offset > 0 ? 1.f / scale : scale);
    const float curDist = camera.view.translation().norm();
    camera.view.matrix()(2, 3) += curDist * alpha;
    return *this;
  }
};

} // namespace candlewick
