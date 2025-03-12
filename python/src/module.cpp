#include "fwd.hpp"
#include "candlewick/config.h"
#include "candlewick/core/Renderer.h"
#include "candlewick/core/Shader.h"

#include <SDL3/SDL_init.h>

using namespace candlewick;

BOOST_PYTHON_FUNCTION_OVERLOADS(adfs_overloads,
                                auto_detect_shader_format_subset, 0, 1)

#ifdef CANDLEWICK_PYTHON_PINOCCHIO_SUPPORT
void exposeVisualizer();
#endif
void exposeRenderer() {
  bp::class_<Device, boost::noncopyable>("Device", bp::no_init)
      .def("driverName", &Device::driverName, ("self"_a))
      .def("shaderFormats", &Device::shaderFormats);

  bp::def("get_num_gpu_drivers", SDL_GetNumGPUDrivers,
          "Get number of available GPU drivers.");

  bp::def("get_gpu_driver_name", SDL_GetGPUDriver, ("index"_a),
          "Get the name of the GPU driver of the corresponding index.");

  bp::def("auto_detect_shader_format_subset", auto_detect_shader_format_subset,
          adfs_overloads{
              ("driver_name"_a = NULL),
              "Automatically detect the compatible set of shader formats."});

  bp::class_<Renderer, boost::noncopyable>("Renderer", bp::no_init)
      .def_readonly("device", &Renderer::device);

  // Register SDL_Quit() as a function to call when interpreter exits.
  Py_AtExit(SDL_Quit);
}

BOOST_PYTHON_MODULE(pycandlewick) {
  bp::import("eigenpy");
  bp::scope current_scope;
  current_scope.attr("__version__") = bp::str(CANDLEWICK_VERSION);

  if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
    throw std::runtime_error(std::format(
        "Failed to initialize SDL subsystems: \'%s\'", SDL_GetError()));
  }
  ::candlewick::setShadersDirectory(SHADERS_INSTALL_DIR);
  bp::def("currentShaderDirectory", &currentShaderDirectory);

  exposeRenderer();
#ifdef CANDLEWICK_PYTHON_PINOCCHIO_SUPPORT
  {
    bp::scope submod = get_namespace("multibody");
    exposeVisualizer();
  }
#endif
}
