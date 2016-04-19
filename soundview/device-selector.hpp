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
#include <vector>

#include "soundview/config.hpp"

namespace soundview {
  typedef std::function<bool()> status_func_t;

  class LIB_API DeviceSelector {
   public:
    DeviceSelector(status_func_t status_cb);

    /**
     * Produces a list of all available audio devices. List may be empty if
     * audio subsystem isn't available, or if no devices are present.
     */
    std::vector<std::string> list_devices();

    /**
     * Looks for a suitable audio device to display, storing the result in
     * 'device'. Loudest device wins. Samples devices for audio, returning the
     * device which is producing the highest amplitudes. Returns `true` and sets
     * 'device' to the device label when a device is found, or gives up and
     * returns false if no recordable devices are available.
     */
    bool auto_select(std::string& device);

   private:
    const status_func_t status_cb;
  };
}
