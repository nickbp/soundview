cmake_minimum_required (VERSION 2.6)

project(soundview)

set(soundview_VERSION_MAJOR 0)
set(soundview_VERSION_MINOR 1)
set(soundview_VERSION_PATCH 0)

# CONFIGURABLE SEARCH PATHS

find_path(sfml_BASE_DIR NAMES include/SFML/Graphics.hpp)
find_path(fftw_BASE_DIR NAMES api/fftw3.h include/fftw3.h fftw3.h)

if(NOT EXISTS ${sfml_BASE_DIR})
  message(ERROR " Configure sfml_BASE_DIR to where SFML is located (should contain bin/, include/, lib/ ..)")
endif()
if(NOT EXISTS ${fftw_BASE_DIR})
  message(ERROR " Configure fftw_BASE_DIR to where FFTW headers/libs are located (should contain fftw3.h and libfftw3 libs)")
endif()
if(NOT EXISTS ${sfml_BASE_DIR} OR NOT EXISTS ${fftw_BASE_DIR})
  message("Exiting")
  return()
endif()

# Libraries and include paths

find_path(fftw_INCLUDE_DIR NAMES fftw3.h HINTS ${fftw_BASE_DIR}/api ${fftw_BASE_DIR}/include ${fftw_BASE_DIR})
find_library(fftw_LIBRARY NAMES libfftw3-3 fftw3 HINTS ${fftw_BASE_DIR}/lib ${fftw_BASE_DIR})

find_path(sfml_INCLUDE_DIR NAMES SFML/Graphics.hpp HINTS ${sfml_BASE_DIR}/include)
find_library(sfml_audio_LIBRARY NAMES sfml-audio HINTS ${sfml_BASE_DIR}/lib)
find_library(sfml_graphics_LIBRARY NAMES sfml-graphics HINTS ${sfml_BASE_DIR}/lib)
find_library(sfml_system_LIBRARY NAMES sfml-system HINTS ${sfml_BASE_DIR}/lib)
find_library(sfml_window_LIBRARY NAMES sfml-window HINTS ${sfml_BASE_DIR}/lib)

# Manually include .dlls in windows install package:
if(WIN32)
  function(copy_include_lib filename hintpath)
    find_file(DLL_${filename} ${filename} HINTS ${hintpath})
    if(EXISTS ${DLL_${filename}})
      install(FILES ${DLL_${filename}} DESTINATION bin)
    else()
      message(ERROR "Didn't find DLL named ${filename} within ${hintpath}: ${DLL_${filename}}")
    endif()
  endfunction()
  copy_include_lib(libfftw3-3.dll ${fftw_BASE_DIR}) # windows: just in base dir
  copy_include_lib(sfml-audio-2.dll ${sfml_BASE_DIR}/bin)
  copy_include_lib(sfml-graphics-2.dll ${sfml_BASE_DIR}/bin)
  copy_include_lib(sfml-system-2.dll ${sfml_BASE_DIR}/bin)
  copy_include_lib(sfml-window-2.dll ${sfml_BASE_DIR}/bin)
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/3rdparty/
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
    set(CMAKE_CXX_FLAGS "/Qstd=c++11 /Qdiag-enable sc")
  else()
    set(CMAKE_CXX_FLAGS "-std=c++11 -diag-enable:sc")
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
  set(CMAKE_CXX_FLAGS "/Wall")
endif()

# Disable some garbage pointless warning
if(APPLE)
  set(CMAKE_MACOSX_RPATH ON)
endif()

add_subdirectory(apps)
add_subdirectory(soundview)

set(CPACK_PACKAGE_NAME "SoundView")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENCE")
set(CPACK_PACKAGE_VERSION_MAJOR "${soundview_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${soundview_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${soundview_VERSION_PATCH}")
set(CPACK_STRIP_FILES true)
set(CPACK_PACKAGE_EXECUTABLES "soundview" "SoundView")
include(CPack)
