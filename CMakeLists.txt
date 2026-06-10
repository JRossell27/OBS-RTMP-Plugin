cmake_minimum_required(VERSION 3.16...3.28)

project(obs-rtmp-receiver VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

set(LIBOBS_INCLUDE_DIR "" CACHE STRING "Semicolon-separated paths to OBS/libobs headers, for example OBS.app/Contents/Frameworks/libobs.framework/Headers")
set(LIBOBS_LIBRARY "" CACHE FILEPATH "Path to the libobs library or framework binary")
set(SIMDE_INCLUDE_DIR "" CACHE PATH "Path to SIMDe headers containing simde/x86/sse2.h; needed by recent OBS/libobs headers on Apple Silicon")

set(OBS_VERSION "unknown" CACHE STRING "OBS version used for generated libobs compatibility headers")
set(OBS_DATA_PATH "../../data" CACHE STRING "OBS data path used for generated libobs compatibility headers")
set(OBS_INSTALL_PREFIX "" CACHE STRING "OBS install prefix used for generated libobs compatibility headers")
set(OBS_PLUGIN_PATH "obs-plugins" CACHE STRING "OBS plugin path used for generated libobs compatibility headers")
set(OBS_PLUGIN_DESTINATION "obs-plugins" CACHE STRING "OBS plugin install destination used for generated libobs compatibility headers")
set(OBS_RELATIVE_PREFIX "../../" CACHE STRING "OBS relative prefix used for generated libobs compatibility headers")
set(OBS_RELEASE_CANDIDATE 0 CACHE STRING "OBS release candidate flag used for generated libobs compatibility headers")
set(OBS_BETA 0 CACHE STRING "OBS beta flag used for generated libobs compatibility headers")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/obsconfig.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/generated/libobs/obsconfig.h"
  @ONLY
)

add_library(obs-rtmp-receiver MODULE
  src/plugin-main.cpp
)

if((LIBOBS_INCLUDE_DIR AND NOT LIBOBS_LIBRARY) OR (LIBOBS_LIBRARY AND NOT LIBOBS_INCLUDE_DIR))
  message(FATAL_ERROR "LIBOBS_INCLUDE_DIR and LIBOBS_LIBRARY must be provided together, or neither should be provided.")
endif()

if(SIMDE_INCLUDE_DIR)
  if(NOT EXISTS "${SIMDE_INCLUDE_DIR}/simde/x86/sse2.h")
    message(FATAL_ERROR "SIMDE_INCLUDE_DIR must contain simde/x86/sse2.h")
  endif()
  target_include_directories(obs-rtmp-receiver PRIVATE "${SIMDE_INCLUDE_DIR}")
else()
  find_path(SIMDE_INCLUDE_DIR_AUTO simde/x86/sse2.h)
  if(SIMDE_INCLUDE_DIR_AUTO)
    target_include_directories(obs-rtmp-receiver PRIVATE "${SIMDE_INCLUDE_DIR_AUTO}")
  endif()
endif()

if(LIBOBS_INCLUDE_DIR AND LIBOBS_LIBRARY)
  foreach(LIBOBS_INCLUDE_PATH IN LISTS LIBOBS_INCLUDE_DIR)
    if(NOT IS_DIRECTORY "${LIBOBS_INCLUDE_PATH}")
      message(FATAL_ERROR "LIBOBS_INCLUDE_DIR entry does not exist: ${LIBOBS_INCLUDE_PATH}")
    endif()
  endforeach()

  target_include_directories(obs-rtmp-receiver PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/generated/libobs"
    ${LIBOBS_INCLUDE_DIR}
  )
  target_link_libraries(obs-rtmp-receiver PRIVATE "${LIBOBS_LIBRARY}")
else()
  find_package(libobs QUIET)

  if(TARGET OBS::libobs)
    target_link_libraries(obs-rtmp-receiver PRIVATE OBS::libobs)
  elseif(TARGET libobs::libobs)
    target_link_libraries(obs-rtmp-receiver PRIVATE libobs::libobs)
  else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBOBS REQUIRED IMPORTED_TARGET libobs)
    target_link_libraries(obs-rtmp-receiver PRIVATE PkgConfig::LIBOBS)
  endif()
endif()

target_compile_definitions(obs-rtmp-receiver PRIVATE
  PLUGIN_NAME="obs-rtmp-receiver"
  PLUGIN_VERSION="${PROJECT_VERSION}"
)

set_target_properties(obs-rtmp-receiver PROPERTIES
  PREFIX ""
  OUTPUT_NAME "obs-rtmp-receiver"
)

if(APPLE)
  set_target_properties(obs-rtmp-receiver PROPERTIES
    BUNDLE TRUE
    BUNDLE_EXTENSION "plugin"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/cmake/macos/Info.plist.in"
  )
endif()

install(TARGETS obs-rtmp-receiver
  BUNDLE DESTINATION "${CMAKE_INSTALL_LIBDIR}/obs-plugins"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/obs-plugins"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/obs-plugins"
)
