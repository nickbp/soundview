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

#include <string>

namespace soundview {

  /**
   * The interface for providing configuration settings to library components.
   */
  class Options {
   public:
    virtual bool list_devices() const = 0;
    virtual std::string device() const = 0;

    virtual size_t audio_collect_rate_hz() const = 0;
    virtual size_t audio_sample_rate_hz() const = 0;

    virtual size_t display_fps_max() const = 0;
    virtual bool display_vsync() const = 0;
    virtual bool display_fullscreen() const = 0;

    virtual size_t bucket_count() const = 0;
    virtual size_t bucket_bass_exaggeration() const = 0;

    virtual size_t color_lum_exaggeration() const = 0;
    virtual size_t color_max_lum() const = 0;

    virtual size_t analyzer_width_pct() const = 0;
    virtual size_t voiceprint_scroll_rate() const = 0;
    virtual size_t loudness_adjust_rate() const = 0;
  };

}
