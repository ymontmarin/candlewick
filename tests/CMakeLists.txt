enable_testing()
find_package(GTest REQUIRED)
find_package(CLI11 CONFIG REQUIRED)

add_library(Common STATIC lib/Common.cpp lib/PerlinNoise.cpp)
target_link_libraries(Common PUBLIC candlewick_core)
target_include_directories(Common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib)

if(BUILD_PINOCCHIO_VISUALIZER)
  target_sources(Common PRIVATE lib/GenHeightfield.cpp)
  target_link_libraries(Common PUBLIC candlewick_multibody)
endif()

function(create_test filename)
  cmake_path(GET filename STEM name)
  add_executable(${name} ${filename})
  foreach(arg ${ARGN})
    message(STATUS "create_test: Linking ${name} to ${arg}")
    target_link_libraries(${name} PRIVATE ${arg})
  endforeach()
endfunction()

create_test(Triangle.cpp Common)
create_test(ColoredCube.cpp Common)
create_test(MeshNormalsRgb.cpp Common)
create_test(LitMesh.cpp Common)

create_test(Ur5.cpp Common CLI11::CLI11)
create_test(Ur5WithSystems.cpp Common CLI11::CLI11)
create_test(Visualizer.cpp Common)

create_test(TestVertexBlob.cpp candlewick_core GTest::gtest_main)
