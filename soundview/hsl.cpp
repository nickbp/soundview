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

#include <math.h>
#include <algorithm>

#include "soundview/hsl.hpp"

namespace {

  const double ONE_SIXTH = 1 / 6.f;
  const double ONE_THIRD = 1 / 3.f;
  const double ONE_HALF = 1 / 2.f;
  const double TWO_THIRDS = 2 / 3.f;

  double hueToRgbValWithP0(const double q, double t) {
    if (t < 0) {
      ++t;
    }
    if (t < ONE_SIXTH) {
      return q * 6 * t;
    } else if (t < ONE_HALF) {
      return q;
    } else if (t < TWO_THIRDS) {
      return q * (TWO_THIRDS - t) * 6;
    } else {
      return 0;
    }
  }

  double hueToRgbValWithQ1(const double p, double t) {
    if (t < 0) {
      ++t;
    }
    if (t < ONE_SIXTH) {
      return p + ((1 - p) * 6 * t);
    } else if (t < ONE_HALF) {
      return 1;
    } else if (t < TWO_THIRDS) {
      return p + ((1 - p) * (TWO_THIRDS - t) * 6);
    } else {
      return p;
    }
  }

  SDL_Color valueToColor(double max_lum, double lum_exponent, double value) {
    double lum = std::min(max_lum, pow(value, lum_exponent));

    double H = ONE_THIRD * (1 - value);
    lum *= 2;
    SDL_Color c;
    c.a = 0xFF;
    if (lum < 1) {
      c.r = hueToRgbValWithP0(lum, H + ONE_THIRD) * 255;
      c.g = hueToRgbValWithP0(lum, H) * 255;
      c.b = hueToRgbValWithP0(lum, H - ONE_THIRD) * 255;
    } else {
      lum -= 1;
      c.r = hueToRgbValWithQ1(lum, H + ONE_THIRD) * 255;
      c.g = hueToRgbValWithQ1(lum, H) * 255;
      c.b = hueToRgbValWithQ1(lum, H - ONE_THIRD) * 255;
    }
    return c;
  }
}

soundview::HSL::HSL(const Options& options) {
  size_t cache_size = 1024;
  double max_lum = options.color_max_lum() / 100.;
  double lum_exponent = 1 - (options.color_lum_exaggeration() / 100.);
  for (size_t i = 0; i < cache_size; ++i) {
    precached_vals.push_back(::valueToColor(max_lum, lum_exponent, i / (double) cache_size));
  }
  precached_vals_size = precached_vals.size();
}

SDL_Color soundview::HSL::valueToColor(double value) const {
  size_t max_index = precached_vals_size - 1;
  size_t index = value * precached_vals_size;
  if (index > max_index) {
    index = max_index;
  }
  return precached_vals[index];
}
