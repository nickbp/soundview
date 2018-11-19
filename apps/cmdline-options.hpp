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

#include <memory>
#include <string>

#include "soundview/options.hpp"

namespace cxxopts {
  class Options;
}

/**
 * Implementation of Options which parses values out of cmdline args.
 */
class CmdlineOptions : public soundview::Options {
 public:
  CmdlineOptions(int argc, char* argv[]);

  bool list_devices() const;
  std::string device() const;

  size_t audio_sample_rate_hz() const;

  size_t display_fps_max() const;
  bool display_vsync() const;
  bool display_fullscreen() const;

  size_t bucket_count() const;
  size_t bucket_bass_exaggeration() const;

  size_t color_lum_exaggeration() const;
  size_t color_max_lum() const;

  size_t analyzer_width_pct() const;
  size_t voiceprint_scroll_rate() const;
  size_t loudness_adjust_rate() const;

 private:
  // would use unique_ptr, but that's incompatible with fwd-decl
  std::shared_ptr<cxxopts::Options> options;
};
