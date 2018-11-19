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

#include <SDL2/SDL_audio.h>

#include "soundview/config.hpp"
#include "soundview/sound-recorder.hpp"

namespace {
  void data_cb(void* userdata, uint8_t* samples, int samples_len) {
    soundview::TransformerBuffer* buf = static_cast<soundview::TransformerBuffer*>(userdata);

    if (config::is_debug()) {
      int64_t sum = 0;
      int16_t val = 0;
      for (int i = 0; i < samples_len; ++i) {
        val = samples[i];
        if (val < 0) {
          sum -= val;
        } else {
          sum += val;
        }
      }
      DEBUG("got %lu samples, sum=%ld", samples_len, sum);
    }
    buf->add(samples, samples_len);
  }
}

soundview::SoundRecorder::SoundRecorder(const Options& options, buf_func_t freq_output_cb)
  : buf(options, freq_output_cb),
    dev(0) {
}

bool soundview::SoundRecorder::start(const std::string& device, size_t sample_rate_hz) {
  if (dev != 0) {
    ERROR("Recording already started for device: %s", device.c_str());
    return false;
  }

  SDL_AudioSpec desired, obtained;
  memset(&desired, 0, sizeof(desired));
  memset(&obtained, 0, sizeof(obtained));

  desired.freq = sample_rate_hz;
  desired.format = buf.expected_sdl_format();
  desired.channels = 1;
  desired.samples = 4096;
  desired.callback = &data_cb;
  desired.userdata = &buf;
  dev = SDL_OpenAudioDevice(
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

  return true;
}

void soundview::SoundRecorder::stop() {
  if (dev == 0) {
    // No-op
    return;
  }

  // Tell device to stop recording.
  SDL_CloseAudioDevice(dev);
  dev = 0;

  buf.reset();
}

