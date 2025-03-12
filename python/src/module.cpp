#include "fwd.hpp"
#include "candlewick/core/Renderer.h"
#include "candlewick/core/Shader.h"

namespace bp = boost::python;
using namespace candlewick;

#ifdef CANDLEWICK_PYTHON_PINOCCHIO_SUPPORT
void exposeVisualizer();
#endif
void exposeRenderer() {
  bp::class_<Device, boost::noncopyable>("Device", bp::no_init)
      .def("driverName", &Device::driverName, ("self"_a));
  bp::class_<Renderer, boost::noncopyable>("Renderer", bp::no_init)
      .def_readonly("device", &Renderer::device);
}

BOOST_PYTHON_MODULE(pycandlewick) {
  bp::import("eigenpy");
  bp::scope current_scope;
  current_scope.attr("__version__") = bp::str(CANDLEWICK_VERSION);

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
