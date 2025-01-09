#include "../core/math_types.h"
#include "../core/matrix_util.h"

#include <Eigen/Geometry>

namespace candlewick {

struct Camera {
  enum ProjectionType { PROJECTION_PERSPECTIVE, PROJECTION_ORTHOGRAPHIC };
  Degf fov{45.f};
  float aspectRatio;
  float nearPlane = 0.1f;
  float farPlane = 100.f;

  Eigen::Isometry3f cameraPose;
  // cached projection matrix
  Eigen::Matrix4f projection;

  bool needsUpdate = false;
  void updateProjection(ProjectionType projType) {
    if (!needsUpdate)
      return;
    switch (projType) {
    case PROJECTION_PERSPECTIVE:
      projection =
          perspectiveFromFov(Radf(fov), aspectRatio, nearPlane, farPlane);
      break;
    case PROJECTION_ORTHOGRAPHIC:
      projection = orthographicMatrix({}, nearPlane, farPlane);
      break;
    }
  }

  Float3 position() const { return cameraPose.translation(); }
  Float3 forward() const { return -cameraPose.linear().col(2); }
  Float3 up() const { return cameraPose.linear().col(1); }
  Float3 right() const { return cameraPose.linear().col(0); }

  Eigen::Matrix4f viewMatrix() const { return cameraPose.inverse().matrix(); }
};

} // namespace candlewick
