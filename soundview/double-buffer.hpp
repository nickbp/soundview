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

#include <vector>

namespace soundview {

  /**
   * Handles swapping between two vector buffers.
   */
  template <typename T>
  class DoubleBuffer {
   public:
    DoubleBuffer()
      : buf_a(), buf_b(), buf_in(&buf_a) { }

    void add(const T& input) {
      buf_in->push_back(input);
    }

    std::vector<T>* get() {
      if (buf_in == &buf_a) {
        // Input is a, output is b

        if (!buf_b.empty()) {
          // Current output still has data
          return &buf_b;
        }

        // Swap buffers and return new output, if any
        // buf_a may be read from while buf_b is getting appends from another thread
        buf_in = &buf_b;
        return &buf_a;
      } else {
        // Input is b, output is a

        if (!buf_a.empty()) {
          // Current output still has data
          return &buf_a;
        }

        // Swap buffers and return new output
        // buf_b may be read from while buf_a is getting appends from another thread
        buf_in = &buf_a;
        return &buf_b;
      }
    }

   private:
    // Swappable buffers. One is input and the other is output.
    std::vector<T> buf_a, buf_b;
    // Selected input buffer. Swaps when output buffer is empty.
    std::vector<T>* buf_in;
  };

}
