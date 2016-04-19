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

#include "soundview/config.hpp"
#include "soundview/display-runner.hpp"
#include "soundview/display-impl.hpp"
#include "soundview/hsl.hpp"

soundview::DisplayRunner::DisplayRunner(const Options& options, reload_device_func_t reload_device_func)
  : display_impl(new DisplayImpl(options, reload_device_func)),
    thread(new std::thread(std::bind(&soundview::DisplayImpl::run, display_impl.get()))) {
}

soundview::DisplayRunner::~DisplayRunner() { }

bool soundview::DisplayRunner::append_freq_data(const std::vector<double>& freq_data) {
  return (display_impl) ? display_impl->append_freq_data(freq_data) : false;
}

bool soundview::DisplayRunner::check_running() {
  return (bool) display_impl;
}

void soundview::DisplayRunner::wait() {
  thread->join();
  thread.reset();
  display_impl.reset();
}

void soundview::DisplayRunner::exit() {
  if (!display_impl) {
    return;
  }
  display_impl->exit();
  wait();
}
