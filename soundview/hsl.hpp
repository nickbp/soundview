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
#include <SDL2/SDL_pixels.h>

#include "soundview/options.hpp"

namespace soundview {

  /**
   * Transforms amplitude values to colors.
   */
  class HSL {
   public:
    HSL(const Options& options);

    /**
     * Given a calculated value and a desired luminosity for that value, returns an Android color
     * code. This is a reduced version of the normal HSL->RGB algorithm; it always has S=1.
     */
    SDL_Color valueToColor(double value) const;

   private:
    std::vector<SDL_Color> precached_vals;
    size_t precached_vals_size;
  };

}
