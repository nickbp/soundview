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
#include "soundview/transformer-buffer.hpp"

#include <fftw3.h>
#include <SDL2/SDL_audio.h>

#ifdef WIN32
// no std::min
#define MIN(x,y) ((x < y) ? x : y)
#else
#define MIN(x,y) (std::min(x,y))
#endif

// Double the size of the fft_plan buffers: Everything past halfway is zero
soundview::TransformerBuffer::TransformerBuffer(
    const Options& options, buf_func_t freq_output_cb)
  : bucket_count(options.bucket_count()),
    buf_pcm(bucket_count * 2, 0),
    buf_pcm_filled(0),
    buf_complex(bucket_count * 2, std::complex<double>(0,0)),
    buf_freq(bucket_count, 0),
    fft_plan(fftw_plan_dft_r2c_1d(
            bucket_count * 2,
            buf_pcm.data(),
            reinterpret_cast<fftw_complex*>(buf_complex.data()),
            0 /* flags */)),
    freq_output_cb(freq_output_cb) {
  if (!fft_plan) {
    ERROR("FFT Plan construction failed");
  }
}

soundview::TransformerBuffer::~TransformerBuffer() {
  fftw_destroy_plan(fft_plan);
  fftw_cleanup();
}

int soundview::TransformerBuffer::expected_sdl_format() {
  return AUDIO_S16;
}

void soundview::TransformerBuffer::add(const uint8_t* samples, size_t samples_len) {
  // append samples to buf. as buf limit is reached (>=0 times), transform and emit transformed
  size_t samples_offset = 0;
  for (;;) {
    size_t copy_size = MIN(
        samples_len - samples_offset, // remaining samples to copy
        buf_pcm.size() - buf_pcm_filled); // remaining space in buffer
    {
      double* out_ptr = buf_pcm.data() + buf_pcm_filled;
      for (size_t i = 0; i < copy_size; ++i) {
        // direct converstion to dbl for fft:
        // TODO input data is 16bit signed ints
        *out_ptr = samples[samples_offset + i];
        ++out_ptr;
      }
    }
    buf_pcm_filled += copy_size;
    if (buf_pcm_filled == buf_pcm.size()) {
      transform_and_flush();
      buf_pcm_filled = 0;
    }
    samples_offset += copy_size;
    if (samples_offset == samples_len) {
      break;
    }
  }
}

void soundview::TransformerBuffer::reset() {
  buf_pcm_filled = 0;
}

void soundview::TransformerBuffer::transform_and_flush() {
  // transform buf_pcm -> buf_complex -> buf_freq, then send buf_freq
  fftw_execute(fft_plan); // converts buf_pcm => buf_complex
  const size_t size = buf_freq.size();
  for (size_t i = 0; i < size; ++i) {
    buf_freq[i] = std::abs(buf_complex[i]);
  }
  freq_output_cb(buf_freq);
}
