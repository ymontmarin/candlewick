set(
  IMGUI_SRC
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_demo.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp
)

add_library(
  candlewick_imgui
  SHARED
  ${IMGUI_SRC}
  imgui_impl_sdl3_extend.cpp
  imgui_sdl_gpu.cpp
)
target_link_libraries(candlewick_imgui PUBLIC SDL3::SDL3 SDL3::Headers)
target_include_directories(
  candlewick_imgui
  PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
