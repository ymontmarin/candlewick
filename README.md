# Candlewick: a WIP renderer based on SDL_gpu

-----

<p align="center" style="font-weight: bold">
  UNDER DEVELOPMENT
</p>

-----

Candlewick is a WIP library for a renderer based on SDL3's new [GPU API](https://wiki.libsdl.org/SDL3/CategoryGPU).


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
  * [robot_descriptions_cpp](https://github.com/ManifoldFR/robot_descriptions_cpp), a suite of loaders for robots (required for some examples)
