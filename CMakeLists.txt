cmake_minimum_required (VERSION 2.6)

project(soundview)

set(soundview_VERSION_MAJOR 0)
set(soundview_VERSION_MINOR 0)
set(soundview_VERSION_PATCH 1)

# Libraries and include paths

find_path(fftw_INCLUDE_DIR NAMES fftw3.h)
find_library(fftw_LIBRARY NAMES fftw3)
if(fftw_INCLUDE_DIR AND fftw_LIBRARY)
  message(STATUS "Found FFTW3")
else()
  message(ERROR "Didn't find FFTW3")
endif()

find_path(sfml_INCLUDE_DIR NAMES SFML/Graphics.hpp)
if(NOT sfml_INCLUDE_DIR)
  message(ERROR "Didn't find SFML include dir")
endif()

find_library(sfml_audio_LIBRARY NAMES sfml-audio)
if(sfml_audio_LIBRARY)
  message(STATUS "Found SFML-Audio")
else()
  message(ERROR "Didn't find SFML-Audio")
endif()

find_library(sfml_graphics_LIBRARY NAMES sfml-graphics)
if(sfml_graphics_LIBRARY)
  message(STATUS "Found SFML-Graphics")
else()
  message(ERROR "Didn't find SFML-Graphics")
endif()

# These are only needed if they aren't automatically included transitively.
set(NEED_ADDITIONAL_SFML (WIN32 OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin"))
if(NEED_ADDITIONAL_SFML)
  find_library(sfml_system_LIBRARY NAMES sfml-system)
  if(sfml_system_LIBRARY)
    message(STATUS "Found SFML-System")
  else()
    message(ERROR "Didn't find SFML-System")
  endif()
  find_library(sfml_window_LIBRARY NAMES sfml-window)
  if(sfml_window_LIBRARY)
    message(STATUS "Found SFML-Window")
  else()
    message(ERROR "Didn't find SFML-Window")
  endif()
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR} # for generated config.hpp
  ${fftw_INCLUDE_DIR}
  ${sfml_INCLUDE_DIR})

# Enable C++11 and more warnings
if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
  set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL Clang)
  set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL Intel)
  if(WIN32)
    set(CMAKE_CXX_FLAGS "/Qstd=c++11 -diag-enable sc")
  else()
    set(CMAKE_CXX_FLAGS "-std=c++11 /Qdiag-enable:sc")
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
  set(CMAKE_CXX_FLAGS "/Wall")
endif()

add_subdirectory(apps)
add_subdirectory(soundview)

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENCE")
set(CPACK_PACKAGE_VERSION_MAJOR "${soundview_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${soundview_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${soundview_VERSION_PATCH}")
include(CPack)
