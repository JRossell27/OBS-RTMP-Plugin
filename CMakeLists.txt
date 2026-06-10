cmake_minimum_required(VERSION 3.16...3.28)

project(obs-rtmp-receiver VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

set(LIBOBS_INCLUDE_DIR "" CACHE PATH "Path to OBS/libobs headers, for example OBS.app/Contents/Frameworks/libobs.framework/Headers")
set(LIBOBS_LIBRARY "" CACHE FILEPATH "Path to the libobs library or framework binary")

find_package(libobs QUIET)
find_package(libobs QUIET)
find_package(libobs REQUIRED)

add_library(obs-rtmp-receiver MODULE
  src/plugin-main.cpp
)

if(LIBOBS_INCLUDE_DIR AND LIBOBS_LIBRARY)
  target_include_directories(obs-rtmp-receiver PRIVATE "${LIBOBS_INCLUDE_DIR}")
  target_link_libraries(obs-rtmp-receiver PRIVATE "${LIBOBS_LIBRARY}")
elseif(TARGET OBS::libobs)
if(TARGET OBS::libobs)
  target_link_libraries(obs-rtmp-receiver PRIVATE OBS::libobs)
elseif(TARGET libobs::libobs)
  target_link_libraries(obs-rtmp-receiver PRIVATE libobs::libobs)
else()
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(LIBOBS REQUIRED IMPORTED_TARGET libobs)
  target_link_libraries(obs-rtmp-receiver PRIVATE PkgConfig::LIBOBS)
else()
  target_link_libraries(obs-rtmp-receiver PRIVATE libobs::libobs)
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
install(TARGETS obs-rtmp-receiver
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/obs-plugins"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/obs-plugins"
)
