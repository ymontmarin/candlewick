enable_testing()
find_package(Eigen3 REQUIRED)

function(create_test filename)
  get_filename_component(name ${filename} NAME_WE)
  add_executable(${name} ${filename})
  target_link_libraries(${name} PRIVATE candlewick_core)
endfunction()

create_test(Triangle.cpp)
create_test(Cube.cpp)
create_test(load_mesh.cpp)
