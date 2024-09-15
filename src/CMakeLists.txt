find_package(SDL3 REQUIRED)
add_library(${PROJECT_NAME}_core SHARED
  )
target_link_libraries(${PROJECT_NAME}_core PUBLIC SDL3::SDL3 SDL3::Headers)
target_include_directories(${PROJECT_NAME}_core PUBLIC ./)
target_include_directories(${PROJECT_NAME}_core PUBLIC ${PROJECT_SOURCE_DIR}/external/SDL_gpu_shadercross)


add_library(${PROJECT_NAME} INTERFACE)
target_link_libraries(${PROJECT_NAME} INTERFACE ${PROJECT_NAME}_core)
