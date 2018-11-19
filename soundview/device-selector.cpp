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

#include <condition_variable>
#include <sstream>

#ifdef WIN32
// think different
#include <windows.h>
#define SLEEP(x) Sleep(x)
#else
#include <unistd.h>
#define SLEEP(x) sleep(x)
#endif

#include <SDL2/SDL_audio.h>

#include "soundview/config.hpp"
#include "soundview/device-selector.hpp"

namespace {

  // number of samples to collect for analysis
  const static size_t SAMPLE_COUNT = 1000;

  /**
   * Used for device auto-selection. Records a sample of PCM data from a given
   * device, and returns the sum amplitudes from that sample. This is used as
   * a measure of how active a given audio device is.
   */
  class AmplitudeSummer {
   public:
    AmplitudeSummer(const std::string& device)
      : device(device),
        samples_left(SAMPLE_COUNT),
        sum_(0),
        mutex(),
        cv_notify(),
        ready_to_stop(false) {
    }

    bool run(size_t &sum) {
      // Single-use only.
      if (ready_to_stop) {
        return false;
      }

      SDL_AudioSpec desired, obtained;
      memset(&desired, 0, sizeof(desired));
      memset(&obtained, 0, sizeof(obtained));

      // Just go with something super basic and low-quality:
      desired.freq = 11025;
      desired.format = AUDIO_U8;
      desired.channels = 1;
      desired.samples = 4096;
      desired.callback = &AmplitudeSummer::data_cb;
      desired.userdata = this;
      SDL_AudioDeviceID dev = SDL_OpenAudioDevice(
          device.c_str(),
          1 /*iscapture*/,
          &desired,
          &obtained,
          0 /*no changes*/);
      if (dev == 0) {
        ERROR("Failed to open device: %s", device.c_str());
        return false;
      }

      // Tell device to start recording, which is what calls data_cb().
      SDL_PauseAudioDevice(dev, 0);

      // Wait for sample processing thread to accumulate enough samples and exit
      {
        std::unique_lock<std::mutex> lock(mutex);
        while (!ready_to_stop) {
          DEBUG("waiting for ready_to_stop");
          cv_notify.wait(lock);
        }
      }

      // Tell device to stop recording.
      SDL_CloseAudioDevice(dev);

      // Produce sample sum
      sum = sum_;
      return true;
    }

   private:
    // Called on separate thread from everything else
    static void data_cb(void* userdata, uint8_t* samples, int samples_len) {
      AmplitudeSummer* as = static_cast<AmplitudeSummer*>(userdata);

      if (as->samples_left == 0) {
        // Drop data if no longer needed
        DEBUG("exiting early, ignoring %lu samples", samples_len);
        return;
      }

      size_t samples_to_get = (samples_len > (int64_t)as->samples_left) ? as->samples_left : samples_len;
      DEBUG("%lu samples left, %lu in this chunk => get %lu samples",
          as->samples_left, samples_len, samples_to_get);
      for (size_t i = 0; i < samples_to_get; ++i) {
        as->sum_ += std::abs(samples[i]);
      }
      as->samples_left -= samples_to_get;
      if (as->samples_left == 0) {
        // Notify other thread to stop sampling
        DEBUG("no more samples needed with sum %lu", as->sum_);
        {
          std::unique_lock<std::mutex> lock(as->mutex);
          DEBUG("notify should_stop");
          as->ready_to_stop = true;
          as->cv_notify.notify_all();
        }
      } else {
        DEBUG("%lu samples left with sum %lu", as->samples_left, as->sum_);
      }
    }

    const std::string device;
    size_t samples_left;
    size_t sum_;

    std::mutex mutex;
    std::condition_variable cv_notify;
    bool ready_to_stop;
  };

}

soundview::DeviceSelector::DeviceSelector(status_func_t status_cb)
  : status_cb(status_cb) { }

std::vector<std::string> soundview::DeviceSelector::list_devices() {
  std::vector<std::string> recording_device_names;
  for (int i = 0; i < SDL_GetNumAudioDevices(1 /* recording */); ++i) {
    recording_device_names.push_back(std::string(SDL_GetAudioDeviceName(i, 1 /* iscapture */)));
  }
  if (recording_device_names.empty()) {
    LOG("No sound devices were found");
  }
  return recording_device_names;
}

bool soundview::DeviceSelector::auto_select(std::string& device) {
  /* iterate over devices and pick one, exiting early if no devices are found.
   * if devices are found, loop until one is producing some audio. */
  std::string best_device;
  size_t best_device_sum = 0;
  for (;;) {
    std::vector<std::string> all_devices = list_devices();
    if (all_devices.empty()) {
      // give up
      return false;
    }
    LOG("Sampling %lu devices for activity:", all_devices.size());
    if (!status_cb()) {
      // user has closed window, give up
      return false;
    }
    size_t num = 0;
    for (const std::string& d : all_devices) {
      LOG("  %lu: %s ..", ++num, d.c_str());
      AmplitudeSummer summer(d);
      size_t sum;
      if (summer.run(sum)) {
        if (sum == 0) {
          LOG("     no data");
          continue;
        } else {
          LOG("     %lu amplitude units", sum);
        }

        if (sum > best_device_sum) {
          // loudest device wins
          best_device = d;
          best_device_sum = sum;
        }
      } else {
        LOG("     sampling FAILED");
      }
    }
    if (best_device_sum > 0) {
      // found at least one device with some non-zero data
      LOG("=> Autoselected device: %s", best_device.c_str());
      device = best_device;
      return true;
    }
    // all devices are muted. sleep for a bit and try again
    LOG("No devices had activity, retrying in");
    for (int i = 5; i > 0; --i) {
      LOG("    %d", i);
      if (!status_cb()) {
        // user has closed window, give up
        return false;
      }
      SLEEP(1);
    }
  }
}
