find_package(SDL3 3.1.3 REQUIRED)
find_package(assimp REQUIRED)
find_package(Eigen3 REQUIRED)
add_library(
  ${PROJECT_NAME}_core
  SHARED
  candlewick/core/Device.cpp
  candlewick/core/Mesh.cpp
  candlewick/core/MeshLayout.cpp
  candlewick/core/MeshLoader.cpp
  candlewick/core/Shader.cpp
  candlewick/core/Texture2D.cpp
)

target_link_libraries(${PROJECT_NAME}_core PUBLIC SDL3::SDL3 SDL3::Headers)
target_link_libraries(${PROJECT_NAME}_core PUBLIC assimp::assimp Eigen3::Eigen)
target_include_directories(${PROJECT_NAME}_core PUBLIC ./)
target_include_directories(
  ${PROJECT_NAME}_core
  PUBLIC
    ${PROJECT_SOURCE_DIR}/external/SDL_gpu_shadercross
    ${PROJECT_SOURCE_DIR}/external/imgui/backends
)

add_library(${PROJECT_NAME} INTERFACE)

cmake_policy(SET CMP0167 NEW)
find_package(pinocchio)
set(BUILD_WITH_PINOCCHIO ${pinocchio_FOUND})
if(${BUILD_WITH_PINOCCHIO})
  add_library(candlewick_multibody SHARED candlewick/multibody/LoadUrdf.cpp)
  target_link_libraries(
    candlewick_multibody
    PUBLIC pinocchio::pinocchio_parsers
  )
  target_link_libraries(${PROJECT_NAME} INTERFACE candlewick_multibody)
endif()

target_link_libraries(${PROJECT_NAME} INTERFACE ${PROJECT_NAME}_core)
target_include_directories(${PROJECT_NAME} INTERFACE candlewick)
