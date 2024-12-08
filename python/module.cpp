#include <eigenpy/eigenpy.hpp>
#include "candlewick/config.h"

namespace bp = boost::python;

BOOST_PYTHON_MODULE(candlewick) {
  bp::scope current_scope;
  current_scope.attr("__version__") = bp::str(CANDLEWICK_VERSION);
}
