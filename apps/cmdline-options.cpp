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

#include <limits>

#include <cxxopts/cxxopts.hpp>

#include "soundview/config.hpp"
#include "apps/cmdline-options.hpp"


// use #define instead of const char* to allow compile-time concat:

#define HELP "help"
#define VERBOSE "verbose"

#define LIST_DEVICES "list-devices"
#define DEVICE "device"

#define AUDIO_COLLECT_RATE "collect-rate"
#define AUDIO_SAMPLE_RATE "sample-rate"

#define FULLSCREEN "fullscreen"
#define VSYNC "vsync"
#define MAX_FPS "fps-max"

#define BUCKET_COUNT "buckets"
#define BUCKET_BASS_EXAGGERATION "bass-width"

#define COLOR_LUM_EXAGGERATION "lum-exaggeration"
#define COLOR_MAX_LUM "max-lum"

#define ANALYZER_WIDTH_PCT "analyzer-width"
#define VOICEPRINT_SCROLL_RATE "voiceprint-scroll"
#define LOUDNESS_ADJUST_RATE "loudness-adjust"


namespace {
  size_t get_uint(cxxopts::Options& options, const char* name,
      size_t min, size_t max = std::numeric_limits<size_t>::max()) {
    size_t val = options[name].as<size_t>();
    if (val < min || val > max) {
      if (max != std::numeric_limits<size_t>::max()) {
        ERROR("Value must be within range [%lu,%lu]: %s = %s", min, max, name, val);
      } else {
        ERROR("Value must be within range [%lu,INT_MAX]: %s = %s", min, name, val);
      }
      exit(1);
    }
    return val;
  }

  std::string get_version() {
    std::ostringstream oss;
    oss << "\n  v" << config::VERSION_STRING << " (" << config::BUILD_DATE << ")";
    return oss.str();
  }
}


CmdlineOptions::CmdlineOptions(int argc, char* argv[])
: options(new cxxopts::Options(argv[0], get_version())) {

  options->add_options()
    ("h," HELP,
        "Displays this message")
    ("v," VERBOSE,
        "Enables verbose logging")
    ;

  options->add_options("Device selection")
    ("l," LIST_DEVICES,
        "Lists all available audio devices, which may be provided to --" DEVICE ".")
    ("d," DEVICE,
        "Overrides the default autodetected device with a name or ID number "
        "produced by --" LIST_DEVICES ".",
        cxxopts::value<std::string>())
    ;

  options->add_options("Audio input")
    (AUDIO_COLLECT_RATE,
        "How frequently to collect blocks of samples produced by the device, in Hz",
        cxxopts::value<size_t>()->default_value("60"))
    (AUDIO_SAMPLE_RATE,
        "Sample rate for the device audio stream, in Hz",
        cxxopts::value<size_t>()->default_value("176400"))
    ;

  options->add_options("Display")
    ("f," FULLSCREEN,
        "Run in fullscreen mode.")
    (VSYNC,
        "Enables VSync for potentially reduced tearing.")
    (MAX_FPS,
        "Maximum FPS to use for the display. Too high just wastes CPU.",
        cxxopts::value<size_t>()->default_value("60"))
    ;

  options->add_options("Appearance")
    (BUCKET_COUNT,
        "How many freq buckets to use in the displayed spectrum.",
        cxxopts::value<size_t>()->default_value("4096"))
    (BUCKET_BASS_EXAGGERATION,
        "How much low/mid freqs should be widened compared to high freqs.",
        cxxopts::value<size_t>()->default_value("80"))

    (COLOR_LUM_EXAGGERATION,
        "How much to exaggerate the luminosity of low values to make them more visible.",
        cxxopts::value<size_t>()->default_value("40"))
    (COLOR_MAX_LUM,
        "Maximum luminosity value to be displayed.",
        cxxopts::value<size_t>()->default_value("50"))

    (ANALYZER_WIDTH_PCT,
        "Width of the spectrum analyzer, as a percentage of the screen.",
        cxxopts::value<size_t>()->default_value("20"))
    (VOICEPRINT_SCROLL_RATE,
        "How quickly voiceprint should scroll.",
        cxxopts::value<size_t>()->default_value("3"))
    (LOUDNESS_ADJUST_RATE,
        "How quickly to recover levels following a loud noise.",
        cxxopts::value<size_t>()->default_value("3"))
    ;

  try {
    options->parse(argc, argv);
  } catch (const cxxopts::OptionException& e) {
    ERROR("Failed to parse options: %s", e.what());
    exit(1);
  }

  // handle --help and --verbose internally:
  if (options->count(HELP)) {
    fprintf(stderr, "%s", options->help(options->groups()).c_str());
    exit(1);
  }
  if (options->count(VERBOSE)) {
    config::enable_debug();
  }
}


bool CmdlineOptions::list_devices() const {
  return (*options)[LIST_DEVICES].as<bool>();
}
std::string CmdlineOptions::device() const {
  return (*options)[DEVICE].as<std::string>();
}

size_t CmdlineOptions::audio_collect_rate_hz() const {
  return get_uint(*options, AUDIO_COLLECT_RATE, 1);
}
size_t CmdlineOptions::audio_sample_rate_hz() const {
  return get_uint(*options, AUDIO_SAMPLE_RATE, 1);
}

size_t CmdlineOptions::display_fps_max() const {
  return get_uint(*options, MAX_FPS, 1);
}
bool CmdlineOptions::display_vsync() const {
  return (*options)[VSYNC].as<bool>();
}
bool CmdlineOptions::display_fullscreen() const {
  return (*options)[FULLSCREEN].as<bool>();
}

size_t CmdlineOptions::bucket_count() const {
  return get_uint(*options, BUCKET_COUNT, 1);
}
size_t CmdlineOptions::bucket_bass_exaggeration() const {
  return get_uint(*options, BUCKET_BASS_EXAGGERATION, 0, 900);
}

size_t CmdlineOptions::color_lum_exaggeration() const {
  return get_uint(*options, COLOR_LUM_EXAGGERATION, 0, 100);
}
size_t CmdlineOptions::color_max_lum() const {
  return get_uint(*options, COLOR_MAX_LUM, 0);
}

size_t CmdlineOptions::analyzer_width_pct() const {
  return get_uint(*options, ANALYZER_WIDTH_PCT, 0, 100);
}
size_t CmdlineOptions::voiceprint_scroll_rate() const {
  return get_uint(*options, VOICEPRINT_SCROLL_RATE, 1);
}
size_t CmdlineOptions::loudness_adjust_rate() const {
  return get_uint(*options, LOUDNESS_ADJUST_RATE, 0);
}
