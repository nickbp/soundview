/* SoundView - Eye candy for your music
 * Copyright (C) 2016 Nicholas Parker
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#pragma once

#include <stdio.h>

/* winders hax */

#ifdef WIN32
#define LIB_API __declspec(dllexport)
#else
#define LIB_API
#endif

/* Some simple print helpers */

#define DEBUG(...) config::_debug(__FUNCTION__, __VA_ARGS__)
#define LOG(...) config::_log(__FUNCTION__, __VA_ARGS__)
#define ERROR(...) config::_error(__FUNCTION__, __VA_ARGS__)

/* Skips "ERR" and func name in output. Used by help output. */
#define PRINT_HELP(...) config::_error(NULL, __VA_ARGS__)

/**
 * Implements logging and version info.
 */
namespace config {
  static const int
  VERSION_MAJOR = @soundview_VERSION_MAJOR@,
    VERSION_MINOR = @soundview_VERSION_MINOR@,
    VERSION_PATCH = @soundview_VERSION_PATCH@;

  static const char VERSION_STRING[] =
    "@soundview_VERSION_MAJOR@.@soundview_VERSION_MINOR@.@soundview_VERSION_PATCH@";
  static const char BUILD_DATE[] = __TIMESTAMP__;

  extern FILE *fout;
  extern FILE *ferr;

  void LIB_API enable_debug();

  /* DONT USE THESE DIRECTLY, use DEBUG()/LOG()/ERROR() instead.
   * The ones with a 'format' function support printf-style format before a list of args.
   * The ones without are for direct unformatted output (eg "_error("func", "printme");") */
  void LIB_API _debug(const char* func, const char* format, ...);
  void LIB_API _debug(const char* func, ...);

  void LIB_API _log(const char* func, const char* format, ...);
  void LIB_API _log(const char* func, ...);
  void LIB_API _error(const char* func, const char* format, ...);
  void LIB_API _error(const char* func, ...);
}
