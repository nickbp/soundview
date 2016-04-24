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

#include <functional>
#include <memory>
#include <thread>
#include <vector>

#include "soundview/config.hpp"
#include "soundview/options.hpp"

namespace soundview {
  class DisplayImpl;

  typedef std::function<bool()> reload_device_func_t;

  /**
   * Wrapper for accepting frequency data and displaying it.
   * Mainly handles threading for an underlying DisplayImpl.
   */
  class LIB_API DisplayRunner {
   public:
    DisplayRunner(const Options& options, reload_device_func_t reload_device_func);
    virtual ~DisplayRunner();

    /**
     * Appends freq data to be displayed.
     */
    bool append_freq_data(const std::vector<double>& freq_data);

    /**
     * Returns whether the display is still running. False = user exited.
     */
    bool check_running();

    /**
     * Blocks until the display has finished running.
     */
    void wait();

    /**
     * Tells the display to stop running.
     */
    void exit();

   private:
    std::unique_ptr<DisplayImpl> display_impl;
    std::unique_ptr<std::thread> thread;
  };
}
