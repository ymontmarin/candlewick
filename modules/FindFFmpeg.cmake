include(FindPackageHandleStandardArgs)

find_package(PkgConfig REQUIRED)
pkg_check_modules(
  FFmpeg
  REQUIRED
  QUIET
  libavdevice
  libavfilter
  libavformat
  libavcodec
  libswresample
  libswscale
  libavutil
)
find_package_handle_standard_args(
  FFmpeg
  FAIL_MESSAGE DEFAULT_MSG
  REQUIRED_VARS FFmpeg_INCLUDE_DIRS FFmpeg_LIBRARIES
)

message(STATUS "FFmpeg libraries: ${FFmpeg_LIBRARIES}")
message(STATUS "FFmpeg library dirs: ${FFmpeg_LIBRARY_DIRS}")
message(STATUS "FFmpeg include dirs: ${FFmpeg_INCLUDE_DIRS}")
message(STATUS "FFmpeg definitions: ${FFmpeg_CFLAGS_OTHER}")

foreach(libname ${FFmpeg_LIBRARIES})
  set(target_name ${libname})
  add_library(${target_name} SHARED IMPORTED)
  find_library(
    libpath_${libname}
    ${libname}
    PATHS ${FFmpeg_LIBRARY_DIRS}
    REQUIRED
  )
  message(STATUS "Component ${libname} found at ${libpath_${libname}}")
  set_target_properties(
    ${target_name}
    PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${FFmpeg_INCLUDE_DIRS}"
      IMPORTED_LOCATION ${libpath_${libname}}
  )

  add_library(FFmpeg::${target_name} ALIAS ${target_name})
endforeach()
