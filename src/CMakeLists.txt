find_package(assimp REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(nlohmann_json 3.11.3 REQUIRED)
find_package(FFmpeg COMPONENTS avformat avcodec swscale)

add_library(
  candlewick_core
  SHARED
  candlewick/core/Camera.cpp
  candlewick/core/DebugScene.cpp
  candlewick/core/Device.cpp
  candlewick/core/math_util.cpp
  candlewick/core/errors.cpp
  candlewick/core/GuiSystem.cpp
  candlewick/core/Mesh.cpp
  candlewick/core/MeshLayout.cpp
  candlewick/core/Renderer.cpp
  candlewick/core/Shader.cpp
  candlewick/core/DepthAndShadowPass.cpp
  candlewick/core/debug/DepthViz.cpp
  candlewick/core/debug/Frustum.cpp
  candlewick/utils/LoadMesh.cpp
  candlewick/utils/LoadMaterial.cpp
  candlewick/utils/MeshData.cpp
  candlewick/utils/MeshDataView.cpp
  candlewick/utils/MeshTransforms.cpp
  candlewick/utils/PixelFormatConversion.cpp
  candlewick/utils/VideoRecorder.cpp
  candlewick/utils/WriteTextureToImage.cpp
  candlewick/primitives/Arrow.cpp
  candlewick/primitives/Cube.cpp
  candlewick/primitives/Grid.cpp
  candlewick/primitives/Heightfield.cpp
  candlewick/primitives/Plane.cpp
  candlewick/third-party/sbti_lib.cpp
)

target_link_libraries(
  candlewick_core
  PUBLIC SDL3::SDL3 SDL3::Headers assimp::assimp Eigen3::Eigen imgui
  PRIVATE nlohmann_json::nlohmann_json
)
set(CANDLEWICK_ASSETS_DIR ${PROJECT_SOURCE_DIR}/assets)
set(CANDLEWICK_SHADERS_DIR ${CANDLEWICK_ASSETS_DIR}/shaders)
target_compile_definitions(
  candlewick_core
  PUBLIC CANDLEWICK_SHADER_BIN_DIR="${CANDLEWICK_SHADERS_DIR}/compiled"
)
target_include_directories(candlewick_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_precompile_headers(candlewick_core PRIVATE <format>)

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
  find_package(pinocchio-visualizers REQUIRED)
  find_package(robot_descriptions_cpp REQUIRED)
  find_package(EnTT REQUIRED)
  add_library(
    candlewick_multibody
    SHARED
    candlewick/multibody/InstantiatePinViz.cpp
    candlewick/multibody/LoadCoalPrimitives.cpp
    candlewick/multibody/LoadPinocchioGeometry.cpp
    candlewick/multibody/RobotDebug.cpp
    candlewick/multibody/RobotScene.cpp
    candlewick/multibody/Visualizer.cpp
  )
  target_link_libraries(
    candlewick_multibody
    PUBLIC
      candlewick_core
      EnTT::EnTT
      robot_descriptions_cpp::robot_descriptions_cpp
      pinocchio::pinocchio_default
      pinocchio-visualizers::pinocchio-visualizers
  )
  target_link_libraries(${PROJECT_NAME} INTERFACE candlewick_multibody)
endif()
