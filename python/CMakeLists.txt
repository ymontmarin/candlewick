include(${PROJECT_SOURCE_DIR}/cmake/python.cmake)

find_package(eigenpy 3.2.0 REQUIRED)

set(PY_SOURCES src/module.cpp src/expose-visualizer.cpp)

set(PYLIB_INSTALL_DIR ${PYTHON_SITELIB}/${PROJECT_NAME})

Python3_add_library(pycandlewick MODULE WITH_SOABI ${PY_SOURCES})
target_link_libraries(pycandlewick PRIVATE candlewick eigenpy::eigenpy)
target_include_directories(
  pycandlewick
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
target_compile_definitions(
  pycandlewick
  PRIVATE SHADERS_INSTALL_DIR="${CANDLEWICK_SHADER_INSTALL_DIR}/compiled"
)

set_target_properties(
  pycandlewick
  PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_NAME}
)
if(UNIX)
  GET_RELATIVE_RPATH(${PYLIB_INSTALL_DIR} PYLIB_INSTALL_RPATH)
  set_target_properties(
    pycandlewick
    PROPERTIES INSTALL_RPATH "${PYLIB_INSTALL_RPATH}"
  )
endif()

install(
  TARGETS pycandlewick
  EXPORT ${TARGETS_EXPORT_NAME}
  LIBRARY DESTINATION ${PYLIB_INSTALL_DIR}
)

set(PYTHON_FILES __init__.py)

foreach(pyfile ${PYTHON_FILES})
  PYTHON_INSTALL_ON_SITE(${PROJECT_NAME} __init__.py)
endforeach()
