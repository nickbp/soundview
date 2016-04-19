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

#include <string.h>

#include <string>
#include <unordered_map>

#include "soundview/options.hpp"

/**
 * Implementation of Options for Windows, which apparently lacks optarg.h. At
 * some point you start to wonder why there should be cmdline arguments on
 * Windows at all. So the Windows version is just locked to some defaults.
 *
 * I'm not wasting my time on this but PRs are welcome.
 */
class DefaultOptions : public soundview::Options {
 public:
  bool list_devices() const { return false; }
  std::string device() const { return ""; }

  size_t audio_collect_rate_hz() const { return 60; }
  size_t audio_sample_rate_hz() const { return 176400; }

  size_t display_fps_max() const { return 60; }
  bool display_vsync() const { return false; }
  bool display_fullscreen() const { return true; }

  size_t bucket_count() const { return 4096;  }
  size_t bucket_bass_exaggeration() const { return 80; }

  size_t color_lum_exaggeration() const { return 40;  }
  size_t color_max_lum() const { return 50;  }

  size_t analyzer_width_pct() const { return 20; }
  size_t voiceprint_scroll_rate() const { return 3; }
  size_t loudness_adjust_rate() const { return 3; }
};
