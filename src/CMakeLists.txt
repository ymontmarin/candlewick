find_package(SDL3 3.1.3 REQUIRED)
find_package(assimp REQUIRED)
find_package(Eigen3 REQUIRED)

set(
  PRIMITIVE_SOURCES
  candlewick/primitives/Cube.cpp
  candlewick/primitives/Plane.cpp
)

add_library(
  ${PROJECT_NAME}_core
  SHARED
  candlewick/core/Device.cpp
  candlewick/core/math_util.cpp
  candlewick/core/Mesh.cpp
  candlewick/core/MeshLayout.cpp
  candlewick/core/Shader.cpp
  candlewick/core/Texture2D.cpp
  candlewick/core/MeshGroup.cpp
  candlewick/utils/LoadMesh.cpp
  candlewick/utils/MaterialData.cpp
  candlewick/utils/MeshData.cpp
  candlewick/utils/MeshDataView.cpp
  candlewick/utils/MeshTransforms.cpp
  ${PRIMITIVE_SOURCES}
)

target_link_libraries(
  ${PROJECT_NAME}_core
  PUBLIC SDL3::SDL3 SDL3::Headers assimp::assimp Eigen3::Eigen
)
target_include_directories(${PROJECT_NAME}_core PUBLIC ./)
target_include_directories(
  ${PROJECT_NAME}_core
  PUBLIC ${PROJECT_SOURCE_DIR}/external/imgui/backends
)

add_library(${PROJECT_NAME} INTERFACE)

cmake_policy(SET CMP0167 NEW)
find_package(pinocchio)
set(BUILD_WITH_PINOCCHIO ${pinocchio_FOUND})
if(${BUILD_WITH_PINOCCHIO})
  find_package(robot_descriptions_cpp REQUIRED)
  add_library(
    candlewick_multibody
    SHARED
    candlewick/multibody/LoadPinocchioGeometry.cpp
  )
  target_link_libraries(
    candlewick_multibody
    PUBLIC pinocchio::pinocchio robot_descriptions_cpp::robot_descriptions_cpp
  )
  target_link_libraries(${PROJECT_NAME} INTERFACE candlewick_multibody)
endif()

target_link_libraries(${PROJECT_NAME} INTERFACE ${PROJECT_NAME}_core)
target_include_directories(${PROJECT_NAME} INTERFACE candlewick)
