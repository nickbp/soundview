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
#include <condition_variable>
#include <limits>

#include <SDL2/SDL.h>

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Window/Event.hpp>

#include "soundview/config.hpp"
#include "soundview/display-impl.hpp"

namespace {
  const char* TITLE = "SoundView";

  /**
   * Resets a region to black, WITHOUT calling target.clear() which introduces flicker
   */
  void reset_region(SDL_Renderer* renderer, SDL_Rect& rect,
      size_t left, size_t right, size_t bottom, size_t top) {
    rect.x = left;
    rect.y = top;
    rect.w = right - left;
    rect.h = top - bottom;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);
  }

  void reset_all(SDL_Renderer* renderer) {
    SDL_Rect rect;
    reset_region(renderer, rect,
        0,// left
        target.getSize().x,// right
        0,// bottom
        target.getSize().y);// top
  }
}

soundview::DisplayImpl::DisplayImpl(const Options& options, reload_device_func_t reload_device_func)
  : analyzer_thickness_pct(options.analyzer_width_pct()),
    fullscreen(options.display_fullscreen()),
    vsync(options.display_vsync()),
    fps_max(options.display_fps_max()),
    bucket_count(options.bucket_count()),
    bucket_bass_exaggeration(options.bucket_bass_exaggeration() / 10.),
    voiceprint_scroll_rate(options.voiceprint_scroll_rate()),
    loudness_adjust_rate(1 - (options.loudness_adjust_rate() / 100.)),
    hsl(options),
    reload_device_func(reload_device_func),
    horiz(false),
    analyzer_thickness(0),
    window_width(0),
    window_height(0),
    bucket_cached_view_size(0),
    voiceprint_edge(0),
    device_max_freq_val(std::numeric_limits<double>::min()),
    shutdown(false) { }

void soundview::DisplayImpl::run() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    ERROR("SDL init failed: %s", SDL_GetError());
    return;
  }

  SDL_Window* window;
  // TODO add to flags?: "| SDL_WINDOW_OPENGL"
  if (fullscreen) {
    // width and height are ignored:
    window = SDL_CreateWindow(
        TITLE,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        0, 0,
        SDL_WINDOW_FULLSCREEN_DESKTOP);
  } else {
    window = SDL_CreateWindow(
        TITLE,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1024, 640,
        SDL_WINDOW_RESIZABLE);
  }
  if (window == NULL) {
    ERROR("SDL window creation failed: %s", SDL_GetError());
    SDL_Quit();
    return;
  }

  uint32_t renderer_flags = SDL_RENDERER_ACCELERATED;
  if (vsync) {
    renderer_flags |= SDL_RENDERER_PRESENTVSYNC;
  }
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, renderer_flags);
  if (renderer == NULL) {
    ERROR("SDL renderer creation failed: %s", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return;
  }

  SDL_DisableScreenSaver();

  //TODO window.setFramerateLimit(fps_max);

  sf::RenderTexture texture;
  if (!handle_resize(window.getView().getSize().x, window.getView().getSize().y, texture)) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return;
  }
  // init to black so that resizes before voiceprint has filled the screen look clean
  reset_all(texture);

  std::vector<std::vector<double> >* freqs = NULL;
  while (1) {
    {
      std::unique_lock<std::mutex> lock(mutex);
      if (shutdown) {
        break;
      }
      // Grab any output from double buffers
      freqs = buf_freqs.get();
    }

    //TODO this loop is prone to stuttering. maybe add a timer to smooth the rate?

    draw_freq_data(window, texture, *freqs);
    SDL_RenderPresent(renderer);
    freqs->clear();
    bool was_resized = handle_user_events(window);
    if (was_resized && !handle_resize(window.getView().getSize().x, window.getView().getSize().y, texture)) {
      break;
    }
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

// The following are all called on a separate thread from run():

bool soundview::DisplayImpl::append_freq_data(const std::vector<double>& freq_data) {
  std::unique_lock<std::mutex> lock(mutex);
  DEBUG("input freqs: %lu", freq_data.size());
  buf_freqs.add(freq_data);
  return !shutdown;
}

bool soundview::DisplayImpl::check_running() {
  std::unique_lock<std::mutex> lock(mutex);
  return !shutdown;
}

void soundview::DisplayImpl::exit() {
  std::unique_lock<std::mutex> lock(mutex);
  shutdown = true;
}

// Private:

bool soundview::DisplayImpl::handle_user_events(sf::RenderWindow& window) {
  SDL_Event event;
  bool resized = false;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        shutdown = true;
        window.close();
        break;
      case SDL_KEYUP:
        switch (event.key.keysym.sym) {
          // TODO also event.key.keysym.mod
          case SDLK_d:
            // [D]evice reload
            device_max_freq_val = std::numeric_limits<double>::min();
            reload_device_func();
            break;

          case SDLK_r:
          case SDLK_SPACE:
            // [R]otate
            horiz = !horiz;
            voiceprint_edge = 0;
            resized = true; // reset sizing to reflect flip
            break;

          // many ways to exit:
          case SDLK_ESCAPE:
          case SDLK_q:
            shutdown = true;
            window.close();
            break;
          case SDLK_c:
            if (event.key.keysym.mod & KMOD_LCTRL
                || event.key.keysym.mod & KMOD_RCTRL) { // ^C
              shutdown = true;
              window.close();
            }
            break;
          case SDLK_F4:
            if (event.key.keysym.mod & KMOD_LALT
                || event.key.keysym.mod & KMOD_RALT) { // Alt+F4
              shutdown = true;
              window.close();
            }
            break;

          default:
            break;
        }
        break;
      case SDL_WINDOWEVENT:
        switch (event.window.event) {
          case SDL_WINDOWEVENT_RESIZED:
            // Fall through
          case SDL_WINDOWEVENT_SIZE_CHANGED:
            // TODO: sizes in event->window.data1 and .data2 (int32_t's)
            resized = true;
            break;
            /* TODO
          case SDL_WINDOWEVENT_CLOSE:
            shutdown = true;
            window.close();
            break;
            */
          default:
            break;
        }
        break;
      default:
        break;
    }
  }
  return resized;
}

void soundview::DisplayImpl::draw_freq_data(
    sf::RenderWindow& window, sf::RenderTexture& texture,
    std::vector<std::vector<double> >& freq_sets) {
  if (freq_sets.empty()) {
    return;
  }

  if (horiz) {
    draw_freq_data_horiz(window, texture, freq_sets);
  } else {
    draw_freq_data_vert(window, texture, freq_sets);
  }
}

void soundview::DisplayImpl::draw_freq_data_horiz(
    sf::RenderWindow& window, sf::RenderTexture& texture,
    std::vector<std::vector<double> >& freq_sets) {
  double bucket_y;
  double val_orig;
  double val_relative;
  size_t i;

  sf::VertexArray quad(sf::Quads, 4);

  // first, draw voiceprint and analyzer to the texture

  const bool voiceprint_enabled = analyzer_thickness_pct < 100;
  if (voiceprint_enabled) {
    // voiceprint

    for (const std::vector<double>& data : freq_sets) { // iterate over columns
      // 0=botleft, 1=botright, 2=topright, 3=topleft
      // left and right stay const for the column. note that 'right' may extend beyond the
      // right edge of the texture, so we need to check for any needed wraparound handling

      // left is straightforward:
      quad[0].position.x = quad[3].position.x = voiceprint_edge;// left
      // right edge needs to check for wraparound:
      size_t new_left_edge =
        (voiceprint_edge + voiceprint_scroll_rate) % (window_width - analyzer_thickness);
      if (new_left_edge < voiceprint_edge) {
        // we're wrapping around the texture. (drawing on left edge of texture is handled below)
        quad[1].position.x = quad[2].position.x = window_width - analyzer_thickness;// right
      } else {
        // all in one contiguous region
        quad[1].position.x = quad[2].position.x = new_left_edge;// right
      }

      bucket_y = window_height;
      for (i = 0; i < data.size(); ++i) {
        // while we're in here, calculate device max amplitude (also used in analyzer below)
        val_orig = data[i];
        if (val_orig > device_max_freq_val) {
          device_max_freq_val = val_orig;
        }
        val_relative = val_orig / device_max_freq_val;

        // 0=botleft, 1=botright, 2=topright, 3=topleft
        quad[0].position.y = quad[1].position.y = bucket_y;// bottom
        bucket_y -= bucket_widths[i];
        quad[2].position.y = quad[3].position.y = bucket_y;// top
        SDL_Color c = hsl.valueToColor(val_relative);
        quad[0].color = quad[1].color = quad[2].color = quad[3].color
          = sf::Color(c.r, c.g, c.b, c.a);
        texture.draw(quad);
      }

      if (new_left_edge < voiceprint_edge && new_left_edge != 0) {
        // we've wrapped around the texture and there's a margin on the left edge to cover.
        // repeat the draw operation for the left edge of the texture
        quad[0].position.x = quad[3].position.x = 0;// left
        quad[1].position.x = quad[2].position.x = new_left_edge;// right

        bucket_y = window_height;
        for (i = 0; i < data.size(); ++i) {
          // 0=botleft, 1=botright, 2=topright, 3=topleft
          quad[0].position.y = quad[1].position.y = bucket_y;// bottom
          bucket_y -= bucket_widths[i];
          quad[2].position.y = quad[3].position.y = bucket_y;// top
          SDL_Color c = hsl.valueToColor(data[i] / device_max_freq_val);
          quad[0].color = quad[1].color = quad[2].color = quad[3].color
            = sf::Color(c.r, c.g, c.b, c.a);
          texture.draw(quad);
        }
      }
      voiceprint_edge = new_left_edge;
    }
  }
  if (analyzer_thickness_pct > 0) {
    // analyzer
    // NOTE: this could be drawn directly to the window instead of reusing voiceprint's texture,
    // but then it slightly misaligns with the voiceprint. to keep things looking tidy we also draw
    // this to the same texture.

    // reset analyzer region to black
    const size_t analyzer_left = window_width - analyzer_thickness;
    reset_region(texture, quad,
        analyzer_left,// left
        analyzer_left + analyzer_thickness,// right
        0,// bottom
        window_height);//top

    // if multiple sets are provided, just render the last/most recent one.
    const std::vector<double>& analyzer_data = freq_sets[freq_sets.size() - 1];
    bucket_y = window_height;
    // 0=botleft, 1=botright, 2=topright, 3=topleft
    quad[0].position.x = quad[3].position.x = analyzer_left;// left (const)
    for (i = 0; i < analyzer_data.size(); ++i) {
      if (!voiceprint_enabled) {
        // normally voiceprint would do this, but it's not running
        val_orig = analyzer_data[i];
        if (val_orig > device_max_freq_val) {
          device_max_freq_val = val_orig;
        }
        val_relative = val_orig / device_max_freq_val;
      } else {
        val_relative = analyzer_data[i] / device_max_freq_val;
      }
      // 0=botleft, 1=botright, 2=topright, 3=topleft
      quad[1].position.x = quad[2].position.x
        = analyzer_left + (analyzer_thickness * val_relative);// right (depends on val)
      quad[0].position.y = quad[1].position.y = bucket_y;// bottom
      bucket_y -= bucket_widths[i];
      quad[2].position.y = quad[3].position.y = bucket_y;// top
      SDL_Color c = hsl.valueToColor(val_relative);
      quad[0].color = quad[1].color = quad[2].color = quad[3].color
        = sf::Color(c.r, c.g, c.b, c.a);
      texture.draw(quad);
    }
  }
  texture.display();

  // second, use sprites to draw regions of the texture to the window

  sf::Sprite sprite(texture.getTexture());
  if (voiceprint_enabled) {
    // paint what's to the right of voiceprint_edge on left edge of the display (oldest data)
    sprite.setTextureRect(
        sf::IntRect(
            voiceprint_edge,// left
            0,// top
            window_width - analyzer_thickness - voiceprint_edge,// width
            window_height));// height
    window.draw(sprite);

    // then paint what's to the left of voiceprint_edge on right edge of the display (newest data)
    sprite.setTextureRect(
        sf::IntRect(
            0,// left
            0,// top
            voiceprint_edge,// width
            window_height));// height
    sprite.setPosition(window_width - analyzer_thickness - voiceprint_edge, 0);
    window.draw(sprite);
  }
  if (analyzer_thickness_pct > 0) {
    // analyzer is simpler than voiceprint, just rendered in-place
    sprite.setTextureRect(
        sf::IntRect(
            window_width - analyzer_thickness,// left
            0,// top
            analyzer_thickness,// width
            window_height));// height
    sprite.setPosition(window_width - analyzer_thickness, 0);
    window.draw(sprite);
  }

  // slowly bring ceiling back to current levels following a loud noise
  device_max_freq_val *= loudness_adjust_rate;
  window.display();
}

void soundview::DisplayImpl::draw_freq_data_vert(
    sf::RenderWindow& window, sf::RenderTexture& texture,
    std::vector<std::vector<double> >& freq_sets) {
  double bucket_x;
  double val_orig;
  double val_relative;
  size_t i;

  sf::VertexArray quad(sf::Quads, 4);

  // first, draw voiceprint and analyzer to the texture

  const bool voiceprint_enabled = analyzer_thickness_pct < 100;
  if (voiceprint_enabled) {
    // voiceprint

    for (const std::vector<double>& data : freq_sets) { // iterate over rows
      // 0=botleft, 1=botright, 2=topright, 3=topleft
      // top and bottom stay const for the row. note that 'right' may extend beyond the
      // top edge of the texture, so we need to check for any needed wraparound handling

      if (voiceprint_edge == 0) {
        voiceprint_edge = window_height;
      }
      // bottom is straightforward:
      quad[0].position.y = quad[1].position.y = voiceprint_edge;// bottom
      // top edge needs to check for wraparound:
      size_t new_top_edge;
      if (voiceprint_edge < (voiceprint_scroll_rate + analyzer_thickness)) {
        // we're wrapping around the texture. (drawing on bottom edge of texture is handled below)
        new_top_edge = window_height - analyzer_thickness - (voiceprint_scroll_rate - voiceprint_edge);
        quad[2].position.y = quad[3].position.y = analyzer_thickness;// top
      } else {
        // all in one contiguous region
        new_top_edge = voiceprint_edge - voiceprint_scroll_rate;
        quad[2].position.y = quad[3].position.y = new_top_edge;// top
      }

      bucket_x = 0;
      for (i = 0; i < data.size(); ++i) {
        // while we're in here, calculate device max amplitude (also used in analyzer below)
        val_orig = data[i];
        if (val_orig > device_max_freq_val) {
          device_max_freq_val = val_orig;
        }
        val_relative = val_orig / device_max_freq_val;

        // 0=botleft, 1=botright, 2=topright, 3=topleft
        quad[0].position.x = quad[3].position.x = bucket_x;// left
        bucket_x += bucket_widths[i];
        quad[1].position.x = quad[2].position.x = bucket_x;// right
        SDL_Color c = hsl.valueToColor(val_relative);
        quad[0].color = quad[1].color = quad[2].color = quad[3].color
          = sf::Color(c.r, c.g, c.b, c.a);
        texture.draw(quad);
      }

      if (voiceprint_edge < (voiceprint_scroll_rate + analyzer_thickness)) {
        // we've wrapped around the texture and there's a margin on the bottom edge to cover.
        // repeat the draw operation for the bottom edge of the texture
        // 0=botleft, 1=botright, 2=topright, 3=topleft
        quad[0].position.y = quad[1].position.y = window_height;// bottom
        quad[2].position.y = quad[3].position.y = new_top_edge;// top

        bucket_x = 0;
        for (i = 0; i < data.size(); ++i) {
          // 0=botleft, 1=botright, 2=topright, 3=topleft
          quad[0].position.x = quad[3].position.x = bucket_x;// left
          bucket_x += bucket_widths[i];
          quad[1].position.x = quad[2].position.x = bucket_x;// right
          SDL_Color c = hsl.valueToColor(data[i] / device_max_freq_val);
          quad[0].color = quad[1].color = quad[2].color = quad[3].color
            = sf::Color(c.r, c.g, c.b, c.a);
          texture.draw(quad);
        }
      }
      voiceprint_edge = new_top_edge;
    }
  }
  if (analyzer_thickness_pct > 0) {
    // analyzer
    // NOTE: this could be drawn directly to the window instead of reusing voiceprint's texture,
    // but then it slightly misaligns with the voiceprint. to keep things looking tidy we also draw
    // this to the same texture.

    // reset analyzer region to black
    reset_region(texture, quad,
        0,// left
        window_width,// right
        analyzer_thickness,// bottom
        0);// top

    // if multiple sets are provided, just render the last/most recent one.
    const std::vector<double>& analyzer_data = freq_sets[freq_sets.size() - 1];
    bucket_x = 0;
    // 0=botleft, 1=botright, 2=topright, 3=topleft
    quad[0].position.y = quad[1].position.y = analyzer_thickness;// bottom (const)
    for (i = 0; i < analyzer_data.size(); ++i) {
      if (!voiceprint_enabled) {
        // normally voiceprint would do this, but it's not running
        val_orig = analyzer_data[i];
        if (val_orig > device_max_freq_val) {
          device_max_freq_val = val_orig;
        }
        val_relative = val_orig / device_max_freq_val;
      } else {
        val_relative = analyzer_data[i] / device_max_freq_val;
      }
      // 0=botleft, 1=botright, 2=topright, 3=topleft
      quad[2].position.y = quad[3].position.y
        = analyzer_thickness - (analyzer_thickness * val_relative);// top (depends on val)
      quad[1].position.x = quad[2].position.x = bucket_x;// left
      bucket_x += bucket_widths[i];
      quad[0].position.x = quad[3].position.x = bucket_x;// right
      SDL_Color c = hsl.valueToColor(val_relative);
      quad[0].color = quad[1].color = quad[2].color = quad[3].color
        = sf::Color(c.r, c.g, c.b, c.a);
      texture.draw(quad);
    }
  }
  texture.display();

  // second, use sprites to draw regions of the texture to the window

  sf::Sprite sprite(texture.getTexture());
  if (analyzer_thickness_pct > 0) {
    // analyzer is simpler than voiceprint, just rendered in-place
    sprite.setTextureRect(
        sf::IntRect(
            0,// left
            0,// top
            window_width,// width
            analyzer_thickness));// height
    window.draw(sprite);
  }
  if (voiceprint_enabled) {
    // paint what's above voiceprint_edge on the bottom edge of the display (the oldest data)
    sprite.setTextureRect(
        sf::IntRect(
            0,// left
            analyzer_thickness,// top
            window_width,// width
            voiceprint_edge - analyzer_thickness));// height
    sprite.setPosition(0, window_height - (voiceprint_edge - analyzer_thickness));
    window.draw(sprite);

    // then paint what's below voiceprint_edge on the top edge of the display (the newest data)
    sprite.setTextureRect(
        sf::IntRect(
            0,// left
            voiceprint_edge,// top
            window_width,// width
            window_height - voiceprint_edge));// height
    sprite.setPosition(0, analyzer_thickness);
    window.draw(sprite);
  }

  // slowly bring ceiling back to current levels following a loud noise
  device_max_freq_val *= loudness_adjust_rate;
  window.display();
}

bool soundview::DisplayImpl::handle_resize(
    size_t width, size_t height, sf::RenderTexture& texture) {
  window_width = width;
  window_height = height;

  if (!texture.create(window_width, window_height)) {
    ERROR("Failed to create texture of width %lu, height %lu",
        window_width, window_height);
    return false;
  }
  reset_all(texture);

  if (horiz) {
    handle_resize_buffers(width, height);
  } else {
    handle_resize_buffers(height, width);
  }
  return true;
}

void soundview::DisplayImpl::handle_resize_buffers(size_t width, size_t height) {
  // analyzer on right edge, with voiceprint getting the remainder
  analyzer_thickness = 0.01 * analyzer_thickness_pct * width;

  // Update scaled bucket widths to window height
  if (height != bucket_cached_view_size) {
    bucket_cached_view_size = height;
    if (bucket_widths.empty()) {
      bucket_widths.resize(bucket_count);
    }

    // Formula:
    //   pxlen = (bucket_count - data_i)^scale / bucket_count^scale
    // Integrate over data_i from 0 to bucket_count:
    //   sum(pxlen) = bucket_count / (scale + 1)
    // Scaled formula:
    //   pxlen = (bucket_count - data_i)^scale * (scale + 1) / bucket_count^(scale + 1)
    const double multiplier =
      height * (bucket_bass_exaggeration + 1)
      / pow(bucket_count, bucket_bass_exaggeration + 1);
    for (size_t data_i = 0; data_i < bucket_count; ++data_i) {
      bucket_widths[data_i] = pow(bucket_count - data_i, bucket_bass_exaggeration) * multiplier;
    }
  }
}
