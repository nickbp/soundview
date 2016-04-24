cmake_minimum_required (VERSION 2.6)

project(apps)

set(SRCS soundview.cpp)
# Windows doesn't have standard getopt.h
if(NOT WIN32)
  list(APPEND SRCS
    cmdline-options.cpp
    cmdline-options.hpp)
endif()
add_executable(soundview-app ${SRCS})

target_link_libraries(soundview-app soundview)
set_target_properties(soundview-app PROPERTIES OUTPUT_NAME soundview)

install(TARGETS soundview-app RUNTIME DESTINATION bin)
