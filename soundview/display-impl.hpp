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

#include <condition_variable>
#include <functional>
#include <vector>

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "soundview/double-buffer.hpp"
#include "soundview/hsl.hpp"
#include "soundview/options.hpp"

namespace soundview {

  typedef std::function<bool()> reload_device_func_t;

  /**
   * Underlying implementation of displaying data to the screen.
   */
  class DisplayImpl {
   public:
    DisplayImpl(const Options& options, reload_device_func_t reload_device_func);

    /**
     * The main display thread. Displays freq data provided by append_freq_data() and responds to
     * user input. Blocks until the display has exited.
     */
    void run();

    // The following are all called on a separate thread from run():

    /**
     * Adds audio data to be displayed.
     */
    bool append_freq_data(const std::vector<double>& freq_data);

    /**
     * Returns whether the display is still running.
     */
    bool check_running();

    /**
     * Tells the display to exit.
     */
    void exit();

   private:
    bool handle_user_events(sf::RenderWindow& window);

    void draw_freq_data(sf::RenderWindow& window, sf::RenderTexture& texture,
        std::vector<std::vector<double> >& freq_sets);
    void draw_freq_data_horiz(sf::RenderWindow& window, sf::RenderTexture& texture,
        std::vector<std::vector<double> >& freq_sets);
    void draw_freq_data_vert(sf::RenderWindow& window, sf::RenderTexture& texture,
        std::vector<std::vector<double> >& freq_sets);

    bool handle_resize(sf::RenderWindow& window, sf::RenderTexture& texture);
    void handle_resize_horiz();
    void handle_resize_vert();

    // from options
    const size_t analyzer_thickness_pct;
    const bool fullscreen;
    const bool vsync;
    const size_t fps_max;
    const size_t bucket_count;
    const double bucket_bass_exaggeration;
    const size_t voiceprint_scroll_rate;
    const double loudness_adjust_rate;

    const HSL hsl;
    const reload_device_func_t reload_device_func;

    bool horiz;
    size_t analyzer_thickness;
    size_t window_width;
    size_t window_height;
    // for each bucket, the width (in px) to display for that bucket.
    std::vector<double> bucket_widths;
    size_t bucket_cached_view_size;
    // the right edge of the voiceprint column thats being written to
    size_t voiceprint_edge;

    std::mutex mutex;

    DoubleBuffer<std::vector<double> > buf_freqs;
    double device_max_freq_val;

    bool shutdown;
  };

}
