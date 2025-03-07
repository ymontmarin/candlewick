enable_testing()
find_package(GTest REQUIRED)
find_package(CLI11 CONFIG REQUIRED)

add_subdirectory(robot_descriptions_cpp)

add_library(Common STATIC lib/Common.cpp lib/PerlinNoise.cpp)
target_link_libraries(Common PUBLIC candlewick_core)
target_include_directories(Common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib)

if(BUILD_PINOCCHIO_VISUALIZER)
  target_sources(Common PRIVATE lib/GenHeightfield.cpp)
  target_link_libraries(Common PUBLIC candlewick_multibody)
endif()

function(add_candlewick_example filename)
  cmake_path(GET filename STEM name)
  add_executable(${name} ${filename})
  foreach(arg ${ARGN})
    message(STATUS "create_test: Linking ${name} to ${arg}")
    target_link_libraries(${name} PRIVATE ${arg})
  endforeach()
endfunction()

add_candlewick_example(Triangle.cpp Common)
add_candlewick_example(ColoredCube.cpp Common)
add_candlewick_example(MeshNormalsRgb.cpp Common)
add_candlewick_example(LitMesh.cpp Common)
if(BUILD_PINOCCHIO_VISUALIZER)
  add_candlewick_example(
    Ur5WithSystems.cpp
    Common
    CLI11::CLI11
    robot_descriptions_cpp
  )
  add_candlewick_example(Visualizer.cpp Common robot_descriptions_cpp)
endif()
add_candlewick_example(TestMeshData.cpp candlewick_core GTest::gtest_main)
