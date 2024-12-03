# Copyright (c) 2024 ManifoldFR

cmake_minimum_required(VERSION 3.20)

set(PROJECT_NAME candlewick)
set(PROJECT_CUSTOM_HEADER_EXTENSION "h")
set(PROJECT_DESCRIPTION "A renderer")
set(PROJECT_CUSTOM_HEADER_DIR)
set(PROJECT_URL "https://github.com/ManifoldFR/candlewick")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CXX_DISABLE_WERROR True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(DOXYGEN_USE_MATHJAX NO)
set(DOXYGEN_USE_TEMPLATE_CSS YES)

include(cmake/base.cmake)
compute_project_args(PROJECT_ARGS LANGUAGES CXX)
project(${PROJECT_NAME} ${PROJECT_ARGS})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/$<CONFIG>/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/$<CONFIG>/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/$<CONFIG>/bin)
set(CMAKE_VERBOSE_MAKEFILE False)

option(BUILD_PINOCCHIO_VISUALIZER "Build the Pinocchio visualizer." ON)
set(INSTALL_DOCUMENTATION OFF CACHE BOOL "" FORCE)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(
    CACHE CMAKE_BUILD_TYPE
    PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
  )
endif()

set(IMGUI_DIR ${PROJECT_SOURCE_DIR}/external/imgui)

cmake_policy(SET CMP0167 NEW)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(external/pinocchio-visualizers)
