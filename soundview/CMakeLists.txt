cmake_minimum_required (VERSION 2.6)

project(src)

configure_file (
  "${PROJECT_SOURCE_DIR}/config.in.hpp"
  "${PROJECT_BINARY_DIR}/config.hpp"
  )

# Header files are just provided for IDEs (particularly VS)
add_library(soundview SHARED
  config.cpp
  ${CMAKE_BINARY_DIR}/soundview/config.hpp
  device-selector.cpp
  device-selector.hpp
  display-impl.cpp
  display-impl.hpp
  display-runner.cpp
  display-runner.hpp
  double-buffer.hpp
  hsl.cpp
  hsl.hpp
  options.hpp
  sound-recorder.cpp
  sound-recorder.hpp
  transformer-buffer.cpp
  transformer-buffer.hpp)

target_link_libraries(soundview
  ${fftw_LIBRARY}
  ${sfml_audio_LIBRARY}
  ${sfml_graphics_LIBRARY}
  ${sfml_system_LIBRARY}
  ${sfml_window_LIBRARY})

install(TARGETS soundview
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib)
