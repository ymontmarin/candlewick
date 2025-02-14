#include "candlewick/multibody/Visualizer.h"

#include <robot_descriptions_cpp/robot_load.hpp>

#include <pinocchio/algorithm/joint-configuration.hpp>
#include <pinocchio/algorithm/geometry.hpp>
#include <chrono>

// #include <CLI/App.hpp>

using namespace candlewick::multibody;
using std::chrono::steady_clock;

int main() {

  pin::Model model;
  pin::GeometryModel geom_model;
  robot_descriptions::loadModelsFromToml("ur.toml", "ur5_gripper", model,
                                         &geom_model, NULL);

  Visualizer visualizer{{1920, 1280}, model, geom_model};
  assert(!visualizer.hasExternalData());

  Eigen::VectorXd q0 = pin::neutral(model);
  Eigen::VectorXd q1 = pin::randomConfiguration(model);

  constexpr double dt = 0.04;
  constexpr std::chrono::duration<double> dt_ms{dt};
  static_assert(std::same_as<decltype(dt_ms.count()), double>);
  double t = 0.;

  while (!visualizer.shouldExit()) {
    const auto now = steady_clock::now();

    double alpha = std::sin(t);
    Eigen::VectorXd q = pin::interpolate(model, q0, q1, alpha);

    visualizer.display(q);
    std::this_thread::sleep_until(now + dt_ms);

    t += dt;
  }
}
