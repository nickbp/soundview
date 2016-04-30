cmake_minimum_required (VERSION 2.6)

project(apps)

add_executable(soundview-app
  soundview.cpp
  cmdline-options.cpp
  cmdline-options.hpp)

target_link_libraries(soundview-app soundview)
set_target_properties(soundview-app PROPERTIES OUTPUT_NAME soundview)

install(TARGETS soundview-app RUNTIME DESTINATION bin)
