set(IMGUI_DIR ${PROJECT_SOURCE_DIR}/external/imgui)

set(
  IMGUI_SRC
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_demo.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp
  ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
  ${IMGUI_DIR}/backends/imgui_impl_sdlgpu3.cpp
)

add_library(imgui SHARED ${IMGUI_SRC})
target_link_libraries(imgui PRIVATE SDL3::SDL3 SDL3::Headers)
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)

add_subdirectory(pinocchio-visualizers)
