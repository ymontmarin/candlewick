enable_testing()
find_package(Eigen3 REQUIRED)
find_package(GTest REQUIRED)

add_library(Common SHARED Common.h Common.cpp)
target_link_libraries(Common PUBLIC candlewick_core)

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
create_test(Ur5.cpp Common candlewick_multibody)

create_test(TestVertexBlob.cpp candlewick_core GTest::gtest_main)
