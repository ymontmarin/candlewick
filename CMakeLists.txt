# Copyright (c) 2024-2025 ManifoldFR

cmake_minimum_required(VERSION 3.20)

set(PROJECT_NAME candlewick)
set(PROJECT_CUSTOM_HEADER_EXTENSION "h")
set(PROJECT_DESCRIPTION "A renderer")
set(PROJECT_CUSTOM_HEADER_DIR)
set(PROJECT_URL "https://github.com/ManifoldFR/candlewick")
set(PROJECT_GENERATED_HEADERS_SKIP_WARNING True)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CXX_DISABLE_WERROR True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(DOXYGEN_USE_MATHJAX NO)
set(DOXYGEN_USE_TEMPLATE_CSS YES)

# The following was copied over from https://github.com/Simple-Robotics/aligator:
# Use BoostConfig module distributed by boost library instead of using FindBoost module distributed
# by CMake
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
  # find_package(SDFormat14) reset this policy (with cmake_minimum_required) and redefine the find_dependency macro.
  # Then, futur call of this macro don't use the right version of this policy.
  # To avoid that, we define the CMP0167 default value.
  set(CMAKE_POLICY_DEFAULT_CMP0167 NEW)
endif()
include(cmake/base.cmake)
COMPUTE_PROJECT_ARGS(PROJECT_ARGS LANGUAGES CXX)
project(${PROJECT_NAME} ${PROJECT_ARGS})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/$<CONFIG>/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/$<CONFIG>/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/$<CONFIG>/bin)
set(CMAKE_VERBOSE_MAKEFILE False)

option(BUILD_PYTHON_BINDINGS "Build Python bindings." OFF)
option(BUILD_PINOCCHIO_VISUALIZER "Build the Pinocchio visualizer." ON)
set(INSTALL_DOCUMENTATION OFF CACHE BOOL "" FORCE)

set(
  CMAKE_MODULE_PATH
  "${PROJECT_SOURCE_DIR}/cmake/find-external/Simde"
  "${PROJECT_SOURCE_DIR}/modules"
  ${CMAKE_MODULE_PATH}
)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(
    CACHE CMAKE_BUILD_TYPE
    PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
  )
endif()

find_package(SDL3 3.1.3 REQUIRED)

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(tests)
if(BUILD_PYTHON_BINDINGS)
  add_subdirectory(python)
endif()
