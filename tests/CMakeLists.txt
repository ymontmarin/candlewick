enable_testing()

function(create_test name filename)
  add_executable(${name} ${filename})
  target_link_libraries(${name} PRIVATE ${PROJECT_NAME})
endfunction()


create_test(sdl_window sdl_window.cpp)
