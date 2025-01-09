find_package(assimp REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(FFmpeg COMPONENTS avformat avcodec swscale)

add_library(
  candlewick_core
  SHARED
  candlewick/core/Device.cpp
  candlewick/core/matrix_util.cpp
  candlewick/core/errors.cpp
  candlewick/core/GuiSystem.cpp
  candlewick/core/Mesh.cpp
  candlewick/core/Renderer.cpp
  candlewick/core/Shader.cpp
  candlewick/core/Shape.cpp
  candlewick/utils/LoadMesh.cpp
  candlewick/utils/MaterialData.cpp
  candlewick/utils/MeshData.cpp
  candlewick/utils/MeshDataView.cpp
  candlewick/utils/MeshTransforms.cpp
  candlewick/utils/PixelFormatConversion.cpp
  candlewick/utils/VideoRecorder.cpp
  candlewick/utils/WriteTextureToImage.cpp
  candlewick/primitives/Cube.cpp
  candlewick/primitives/Heightfield.cpp
  candlewick/primitives/Grid.cpp
  candlewick/primitives/Plane.cpp
  candlewick/third-party/sbti_lib.cpp
)

target_link_libraries(
  candlewick_core
  PUBLIC SDL3::SDL3 SDL3::Headers assimp::assimp Eigen3::Eigen imgui
)
target_include_directories(candlewick_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(FFmpeg_FOUND)
  message(STATUS "FFmpeg found. Building video module.")
  target_compile_definitions(candlewick_core PUBLIC CANDLEWICK_HAS_FFMPEG)
  target_link_libraries(
    candlewick_core
    PRIVATE FFmpeg::avformat FFmpeg::avcodec FFmpeg::swscale
  )
  target_sources(candlewick_core PRIVATE candlewick/utils/VideoRecorder.cpp)
endif()

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
      candlewick_core
      robot_descriptions_cpp::robot_descriptions_cpp
      pinocchio::pinocchio_default
      pinocchio-visualizers
  )
  target_link_libraries(${PROJECT_NAME} INTERFACE candlewick_multibody)
endif()
