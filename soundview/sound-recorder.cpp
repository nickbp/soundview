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
#include "soundview/sound-recorder.hpp"

soundview::SoundRecorder::SoundRecorder(const Options& options, buf_func_t freq_output_cb)
  : buf(options, freq_output_cb) {
  auto period = sf::seconds(1 / ((double)options.audio_collect_rate_hz()));
  setProcessingInterval(period);
}

bool soundview::SoundRecorder::onProcessSamples(const int16_t* samples, size_t samples_len) {
  int64_t sum = 0;
  int16_t val = 0;
  for (size_t i = 0; i < samples_len; ++i) {
    val = samples[i];
    if (val < 0) {
      sum -= val;
    } else {
      sum += val;
    }
  }
  DEBUG("got %lu samples, sum=%ld", samples_len, sum);
  buf.add(samples, samples_len);
  return true;
}

void soundview::SoundRecorder::onStop() {
  buf.reset();
}

