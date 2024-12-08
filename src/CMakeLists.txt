find_package(SDL3 3.1.3 REQUIRED)
find_package(assimp REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(FFmpeg COMPONENTS avformat avcodec swscale)

set(
  PRIMITIVE_SOURCES
  candlewick/primitives/Cube.cpp
  candlewick/primitives/Grid.cpp
  candlewick/primitives/Plane.cpp
)

add_library(
  candlewick_core
  SHARED
  candlewick/core/Device.cpp
  candlewick/core/matrix_util.cpp
  candlewick/core/Mesh.cpp
  candlewick/core/Shader.cpp
  candlewick/core/Texture2D.cpp
  candlewick/core/MeshGroup.cpp
  candlewick/utils/LoadMesh.cpp
  candlewick/utils/MeshData.cpp
  candlewick/utils/MeshDataView.cpp
  candlewick/utils/MeshTransforms.cpp
  candlewick/utils/PixelFormatConversion.cpp
  candlewick/utils/VideoRecorder.cpp
  candlewick/utils/WriteTextureToImage.cpp
  ${PRIMITIVE_SOURCES}
  candlewick/third-party/sbti_lib.cpp
)

target_link_libraries(
  candlewick_core
  PUBLIC SDL3::SDL3 SDL3::Headers assimp::assimp Eigen3::Eigen
)

if(FFmpeg_FOUND)
  message(STATUS "FFmpeg found. Building video module.")
  target_compile_definitions(candlewick_core PUBLIC CANDLEWICK_HAS_FFMPEG)
  target_link_libraries(
    candlewick_core
    PRIVATE FFmpeg::avformat FFmpeg::avcodec FFmpeg::swscale
  )
  target_sources(candlewick_core PRIVATE candlewick/utils/VideoRecorder.cpp)
endif()

target_include_directories(candlewick_core PUBLIC ./)

add_subdirectory(candlewick/gui)
target_link_libraries(candlewick_core PUBLIC candlewick_imgui)

add_library(${PROJECT_NAME} INTERFACE)
target_link_libraries(${PROJECT_NAME} INTERFACE candlewick_core)
target_include_directories(${PROJECT_NAME} INTERFACE candlewick)

if(BUILD_PINOCCHIO_VISUALIZER)
  find_package(pinocchio REQUIRED)
  find_package(robot_descriptions_cpp REQUIRED)
  add_library(
    candlewick_multibody
    SHARED
    candlewick/multibody/InstantiatePinViz.cpp
    candlewick/multibody/LoadCoalPrimitives.cpp
    candlewick/multibody/LoadPinocchioGeometry.cpp
    candlewick/multibody/Visualizer.cpp
  )
  target_link_libraries(
    candlewick_multibody
    PUBLIC
      pinocchio::pinocchio
      robot_descriptions_cpp::robot_descriptions_cpp
      pinocchio-visualizers
  )
  target_link_libraries(${PROJECT_NAME} INTERFACE candlewick_multibody)
endif()
