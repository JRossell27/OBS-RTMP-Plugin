cmake_minimum_required(VERSION 3.16...3.28)

project(obs-rtmp-receiver VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

find_package(libobs REQUIRED)

add_library(obs-rtmp-receiver MODULE
  src/plugin-main.cpp
)

if(TARGET OBS::libobs)
  target_link_libraries(obs-rtmp-receiver PRIVATE OBS::libobs)
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

install(TARGETS obs-rtmp-receiver
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/obs-plugins"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/obs-plugins"
)
