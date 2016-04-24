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

#include <complex>
#include <functional>
#include <memory>
#include <vector>

#include "soundview/config.hpp"
#include "soundview/options.hpp"

struct fftw_plan_s;

namespace soundview {

  typedef std::function<void(const std::vector<double>&)> buf_func_t;

  /**
   * Transfroms PCM data to frequency data (via FFT)
   */
  class LIB_API TransformerBuffer {
   public:
    TransformerBuffer(const Options& options, buf_func_t freq_output_cb);
    virtual ~TransformerBuffer();

    void add(const int16_t* samples, size_t samples_len);
    void reset();

   private:
    void transform_and_flush();

    const size_t bucket_count;
    // fixed-size buffer containing pcm data from device
    std::vector<double> buf_pcm;
    // number of used elements in buf_pcm
    size_t buf_pcm_filled;
    // fixed-size buffer containing raw FFT of buf_pcm
    std::vector<std::complex<double>> buf_complex;
    // fixed-size buffer containing magnitudes derived from buf_complex
    std::vector<double> buf_freq;

    fftw_plan_s* fft_plan;
    buf_func_t freq_output_cb;
  };

}
