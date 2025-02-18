import pinocchio as pin
import candlewick as cdw
from candlewick.multibody import Visualizer, VisualizerConfig

import go2_description as go2d
import os.path as osp


print(f"Current shader directory: {cdw.currentShaderDirectory()}")

builder = pin.RobotWrapper.BuildFromURDF
package_dir = osp.realpath(osp.join(go2d.GO2_DESCRIPTION_PACKAGE_DIR, ".."))
print(package_dir)

robot = builder(go2d.GO2_DESCRIPTION_URDF_PATH, [package_dir])
print("Load q0:")
pin.loadReferenceConfigurations(robot.model, go2d.GO2_DESCRIPTION_SRDF_PATH)

rmodel = robot.model
visual_model = robot.visual_model

print(rmodel)
print(visual_model)

config = VisualizerConfig()
config.width = 1600
config.height = 900
viz = Visualizer(config, rmodel, visual_model)

print(
    "Visualizer has renderer:",
    viz.renderer,
    "driver name:",
    viz.renderer.device.driverName(),
)

q0 = rmodel.referenceConfigurations["standing"]
pin.framesForwardKinematics(rmodel, robot.data, q0)
viz.display(q0)
