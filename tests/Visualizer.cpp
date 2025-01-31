#include "candlewick/multibody/Visualizer.h"

#include <robot_descriptions_cpp/robot_load.hpp>

#include <pinocchio/algorithm/joint-configuration.hpp>
#include <pinocchio/algorithm/geometry.hpp>

// #include <CLI/App.hpp>

using namespace candlewick::multibody;

int main(int argc, char *argv[]) {

  pin::Model model;
  pin::GeometryModel geom_model;
  robot_descriptions::loadModelsFromToml("ur.toml", "ur5_gripper", model,
                                         &geom_model, NULL);

  Visualizer visualizer{{1920, 1280}, model, geom_model};

  Eigen::VectorXd q0 = pin::neutral(model);
  Eigen::VectorXd q1 = pin::randomConfiguration(model);
  const double dt = 0.04;

  double t = 0.;

  while (!visualizer.shouldExit()) {

    double alpha = std::sin(t);
    Eigen::VectorXd q = pin::interpolate(model, q0, q1, alpha);

    visualizer.display(q);

    t += dt;
  }
}
