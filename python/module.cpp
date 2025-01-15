#include "fwd.hpp"
#include "candlewick/core/Renderer.h"

namespace bp = boost::python;
using namespace candlewick;

void exposeVisualizer();
void exposeRenderer() {
  bp::class_<Renderer, boost::noncopyable>("Renderer", bp::no_init);
}

BOOST_PYTHON_MODULE(pycandlewick) {
  bp::import("eigenpy");
  bp::scope current_scope;
  current_scope.attr("__version__") = bp::str(CANDLEWICK_VERSION);

  exposeRenderer();
  {
    bp::scope submod = get_namespace("multibody");
    exposeVisualizer();
  }
}
