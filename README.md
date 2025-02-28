# Candlewick: a WIP renderer based on SDL_gpu

-----

<p align="center" style="font-weight: bold">
  UNDER DEVELOPMENT
</p>

-----

Candlewick is a WIP library for a renderer based on SDL3's new [GPU API](https://wiki.libsdl.org/SDL3/CategoryGPU).

## Features

Candlewick comes with a set of graphical, interaction, and utility features.

### Graphical features

* Shadow mapping using directional shadow maps
* Screen-space ambient occlusion (SSAO)
* **WIP:** Screen-space shadows (SSS)

### Interactivitity

* Integration with ImGui
* **(optional)** Record videos from the main window using FFmpeg

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
* [nlohmann/json](https://github.com/nlohmann/json) for processing JSON files
* the [Eigen](https://gitlab.com/libeigen/eigen/) linear algebra template library
* [Open Asset Importer Library (assimp)](https://assimp-docs.readthedocs.io/en/latest/) for loading meshes

These dependencies can be installed from Conda as follows:
```bash
conda install sdl3 eigen assimp entt nlohmann_json
```

#### Optional dependencies

* [eigenpy](https://github.com/stack-of-tasks/eigenpy) for Python bindings.
* [FFmpeg](https://ffmpeg.org/) for support for recording videos from the rendered graphics.
* [GoogleTest](https://github.com/google/googletest) for tests | [conda-forge](https://anaconda.org/conda-forge/gtest)
* For loading and visualizing robots:
  * the [Pinocchio](https://github.com/stack-of-tasks/pinocchio) rigid-body dynamics library (required for the `candlewick::multibody` classes and functions) | [conda-forge](https://anaconda.org/conda-forge/pinocchio)
  * [robot_descriptions_cpp](https://github.com/ManifoldFR/robot_descriptions_cpp), a suite of loaders for robots (required for some examples)

## Credits

Many of the design choices of this library are heavily inspired by other, more mature work in the open-source 3D graphics middleware space.

Here are some of the resources I've looked at:

* the [Magnum](https://magnum.graphics/) graphics middleware (the design ideas around mesh layouts, how to load geometry primitives, and the type-erased `MeshData` type)
* [bgfx](https://github.com/bkaradzic/bgfx/)
* Sascha Willems' Vulkan examples: https://github.com/SaschaWillems/Vulkan/
