#include "../core/math_types.h"
#include "CameraControl.h"

#include <Eigen/Geometry>

namespace candlewick {

struct Camera {
  Eigen::Isometry3f cameraPose;
  Mat4f projection;

  Float3 position() const { return cameraPose.translation(); }
  Float3 forward() const { return -cameraPose.linear().col(2); }
  Float3 up() const { return cameraPose.linear().col(1); }
  Float3 right() const { return cameraPose.linear().col(0); }

  Eigen::Matrix4f viewMatrix() const { return cameraPose.inverse().matrix(); }
  void setPoseFromView(Mat4f view) { cameraPose.matrix() = view.inverse(); }
};

} // namespace candlewick
