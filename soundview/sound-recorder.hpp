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

#include <memory>

#include <SFML/Audio/SoundRecorder.hpp>

#include "soundview/transformer-buffer.hpp"

namespace soundview {

  /**
   * Implementation for retrieving audio samples from a device.
   */
  class LIB_API SoundRecorder : public sf::SoundRecorder {
   public:
    SoundRecorder(const Options& options, buf_func_t freq_output_cb);

   protected:
    bool onProcessSamples(const int16_t* samples, size_t samples_len);
    void onStop();

   private:
    TransformerBuffer buf;
  };

}
