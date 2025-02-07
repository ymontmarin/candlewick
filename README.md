# Candlewick: a WIP renderer based on SDL_gpu

-----

<p align="center" style="font-weight: bold">
  UNDER DEVELOPMENT
</p>

-----

Candlewick is a WIP library for a renderer based on SDL3's new [GPU API](https://wiki.libsdl.org/SDL3/CategoryGPU).

## Features

Candlewick comes with some basic graphical features.

* Shadow mapping using directional shadow maps
* Screen-space ambient occlusion (SSAO)
* **WIP:** Screen-space shadows (SSS)

### Pinocchio visualizer

Candlewick visualization utilities for robotics based on Pinocchio.

You can load a Pinocchio model, its geometry model, and create a visualizer that can be used similar to the other visualizers included in `pinocchio.visualize`.
Here's a Python example:

```python
import example_robot_data as erd
import pinocchio as pin
import numpy as np
from candlewick.multibody import Visualizer, VisualizerConfig

robot = erd.load("ur10")
model: pin.Model = robot.model
data: pin.Data = robot.data
visual_model = robot.visual_model

config = VisualizerConfig()
config.width = 1280
config.height = 720
viz = Visualizer(config, model, visual_model)

q0 = pin.neutral(model)
viz.setCameraPose(pin.SE3.Identity())
viz.display(q0)
```


## Installation

### Dependencies

Candlewick depends mainly on:

* [SDL3](https://github.com/libsdl-org/SDL/) for windowing, processing input events, and of course SDL GPU.
* [EnTT](https://github.com/skypjack/entt/) for the entity-component system (ECS)

#### Optional dependencies

* [eigenpy](https://github.com/stack-of-tasks/eigenpy) for Python bindings.
* [FFmpeg](https://ffmpeg.org/) for support for recording videos from the rendered graphics.
* For loading and visualizing robots:
  * the Pinocchio rigid-body dynamics library (required for the `candlewick::multibody` classes and functions).
  * the [pinocchio-visualizers](https://github.com/Simple-Robotics/pinocchio-visualizers) header-only library, which provides a skeleton for creating a C++ visualizer for Pinocchio and a visitor used for Python bindings
  * [robot_descriptions_cpp](https://github.com/ManifoldFR/robot_descriptions_cpp), a suite of loaders for robots (required for some examples)
