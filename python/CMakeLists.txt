include(${PROJECT_SOURCE_DIR}/cmake/python.cmake)

find_package(eigenpy 3.2.0 REQUIRED)

set(PY_SOURCES module.cpp)

python3_add_library(pycandlewick MODULE WITH_SOABI ${PY_SOURCES})
target_link_libraries(pycandlewick PRIVATE candlewick eigenpy::eigenpy)
target_include_directories(
  pycandlewick
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
