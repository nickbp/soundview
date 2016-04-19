cmake_minimum_required (VERSION 2.6)

project(src)

configure_file (
  "${PROJECT_SOURCE_DIR}/config.in.hpp"
  "${PROJECT_BINARY_DIR}/config.hpp"
  )
add_library(soundview SHARED
  config.cpp
  device-selector.cpp
  display-impl.cpp
  display-runner.cpp
  hsl.cpp
  sound-recorder.cpp
  transformer-buffer.cpp)

target_link_libraries(soundview
  ${fftw_LIBRARY}
  ${sfml_audio_LIBRARY}
  ${sfml_graphics_LIBRARY})