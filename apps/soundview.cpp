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

#include "apps/cmdline-options.hpp"
#include "soundview/config.hpp"
#include "soundview/device-selector.hpp"
#include "soundview/display-runner.hpp"
#include "soundview/sound-recorder.hpp"

namespace sp = std::placeholders;

namespace {
  /**
   * Callback handler for reloading devices
   */
  class DeviceReloader {
   public:
    DeviceReloader(const soundview::Options& options)
      : sample_rate_hz(options.audio_sample_rate_hz()),
        options_device(options.device()),
        recorder(NULL),
        selector(NULL) { }

    void set_selector(soundview::DeviceSelector* selector) {
      this->selector = selector;
    }

    void set_recorder(soundview::SoundRecorder* recorder) {
      this->recorder = recorder;
    }

    bool reload() {
      if (recorder == NULL || selector == NULL) {
        return false;
      }

      // no-op if recorder isn't running yet:
      recorder->stop();

      std::string selected_device = options_device;
      if (!selected_device.empty()) {
        // try to parse specified device as an int index, and map to a device name
        char* invalid_start = NULL;
        size_t index = strtoul(selected_device.c_str(), &invalid_start, 10);
        // check that no invalid data was provided, and display ids start at 1
        if ((invalid_start == NULL || *invalid_start == '\0') && index > 0) {
          --index; // convert to zero-index
          std::vector<std::string> available_devices = selector->list_devices();
          if (index < available_devices.size()) {
            selected_device = available_devices[index];
            LOG("=> Device %lu: %s", (index + 1), selected_device.c_str());
          }
        }
      } else {
        // no device specified in args: auto-detect
        if (!selector->auto_select(selected_device)) {
          return false;
        }
      }

      return recorder->start(selected_device, sample_rate_hz);
    }

   private:
    const size_t sample_rate_hz;
    const std::string options_device;
    soundview::SoundRecorder* recorder;
    soundview::DeviceSelector* selector;
  };
}

int main(int argc, char* argv[]) {
  CmdlineOptions options(argc, argv);

  DeviceReloader reloader(options);

  soundview::DisplayRunner display_runner(options, std::bind(&::DeviceReloader::reload, &reloader));

  soundview::DeviceSelector selector(
      std::bind(&soundview::DisplayRunner::check_running, &display_runner));
  reloader.set_selector(&selector);

  if (options.list_devices()) {
    printf("Available devices:\n");
    std::vector<std::string> devices = selector.list_devices();
    for (size_t i = 0; i < devices.size(); ++i) {
      printf("  %lu: '%s'\n", (i + 1), devices[i].c_str());
    }
    exit(0);
  }

  soundview::SoundRecorder recorder(options,
      std::bind(&soundview::DisplayRunner::append_freq_data, &display_runner, sp::_1));
  reloader.set_recorder(&recorder);

  if (reloader.reload()) {
    display_runner.run();
    LOG("Exiting.");
    recorder.stop();
    return 0;
  } else {
    LOG("Failed to start sound recorder. Exiting.");
    recorder.stop();
    return -1;
  }
}
