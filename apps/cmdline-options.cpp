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

#include <getopt.h>

#include <limits>
#include <sstream>

#include "soundview/config.hpp"
#include "apps/cmdline-options.hpp"

// use #define instead of const char* to allow compile-time concat
#define HELP "help"
#define VERBOSE "verbose"

#define LIST_DEVICES "list-devices"
#define DEVICE "device"

#define AUDIO_COLLECT_RATE "audio-collect-rate"
#define AUDIO_SAMPLE_RATE "audio-sample-rate"

#define FULLSCREEN "fullscreen"
#define VSYNC "vsync"
#define MAX_FPS "max-fps"

#define BUCKET_COUNT "bucket-count"
#define BUCKET_BASS_EXAGGERATION "bucket-bass-exaggeration"

#define COLOR_LUM_EXAGGERATION "color-lum-exaggeration"
#define COLOR_MAX_LUM "color-max-lum"

#define ANALYZER_WIDTH_PCT "analyzer-width-pct"
#define VOICEPRINT_SCROLL_RATE "voiceprint-scroll-rate"
#define LOUDNESS_ADJUST_RATE "loudness-adjust-rate"

namespace {
  enum Type {
    NONE,
    BOOL,
    STRING,
    UNSIGNED,
    DOUBLE
  };
  struct option_info {
    const char* help_msg;
    const char* default_val;

    static option_info stub() {
      return {NULL, NULL};
    }
    // no arg
    static option_info create(const char* help_msg) {
      return {help_msg, NULL};
    }
    // with arg (and default val)
    static option_info create(const char* help_msg, const char* val) {
      return {help_msg, val};
    }
  };

  // MUST MATCH ORDER/CONTENT OF 'option_infos' BELOW
  // char* name, int has_arg, int* flag, int val
  const struct option options[] = {
    {HELP, no_argument, NULL, 'h'},
    {VERBOSE, no_argument, NULL, 'v'},

    {LIST_DEVICES, no_argument, NULL, 'l'},
    {DEVICE, required_argument, NULL, 'd'},

    {AUDIO_COLLECT_RATE, required_argument, NULL, 0},
    {AUDIO_SAMPLE_RATE, required_argument, NULL, 0},

    {FULLSCREEN, no_argument, NULL, 'f'},
    {VSYNC, required_argument, NULL, 0},
    {MAX_FPS, required_argument, NULL, 0},

    {BUCKET_COUNT, required_argument, NULL, 0},
    {BUCKET_BASS_EXAGGERATION, required_argument, NULL, 0},

    {COLOR_LUM_EXAGGERATION, required_argument, NULL, 0},
    {COLOR_MAX_LUM, required_argument, NULL, 0},

    {ANALYZER_WIDTH_PCT, required_argument, NULL, 0},
    {VOICEPRINT_SCROLL_RATE, required_argument, NULL, 0},
    {LOUDNESS_ADJUST_RATE, required_argument, NULL, 0},

    {NULL, 0, NULL, 0}
  };

  // MUST MATCH ORDER/CONTENT OF 'options' ABOVE
  const struct option_info option_infos[] = {
    option_info::create("Displays this message"),
    option_info::create("Enables verbose logging"),

    option_info::create("Lists all available audio devices, which may be provided to --" DEVICE "."),
    option_info::create("Overrides the default autodetected device with a name or id number provided by --" LIST_DEVICES "."),

    option_info::create("How frequently to collect blocks of samples produced by the device, in Hz", "60"),
    option_info::create("Sample rate for the device audio stream, in Hz", "176400"),

    option_info::create("Start in fullscreen mode."),
    option_info::create("Whether to enable VSync for potentially reduced tearing."),
    option_info::create("Maximum FPS to use for the display. Too high just wastes CPU.", "60"),

    option_info::create("How many freq buckets to use in the displayed spectrum.", "4096"),
    option_info::create("How much low/mid freqs should be widened compared to high freqs.", "80"),

    option_info::create("How much to exaggerate the luminosity of low values to make them more visible.", "40"),
    option_info::create("Maximum luminosity value to be displayed.", "50"),

    option_info::create("Width of the spectrum analyzer, as a percentage of the screen.", "20"),
    option_info::create("How quickly voiceprint should scroll.", "3"),
    option_info::create("How quickly to recover levels following a loud noise.", "3"),

    option_info::stub()
  };

  const option& get_option(char c) {
    size_t i = 0;
    for (;; ++i) {
      if (options[i].name == NULL) {
        // got to end without a match
        break;
      }
      if (options[i].val == c) {
        return options[i];
      }
    }
    // return last item, which is a stub.
    // shouldn't happen in practice; implies bad char
    return options[i];
  }

  const option_info& get_info(const std::string& name) {
    size_t i = 0;
    for (;; ++i) {
      if (options[i].name == NULL) {
        // got to end without a match
        break;
      }
      if (strcmp(options[i].name, name.c_str()) == 0) {
        return option_infos[i];
      }
    }
    // return last item, which is a stub.
    // shouldn't happen in practice; implies bad name
    return option_infos[i];
  }

  typedef CmdlineOptions::args_t::const_iterator arg_iter_t;
  const char* pick_val(const CmdlineOptions::args_t& args, const std::string& name) {
    arg_iter_t iter = args.find(name);
    if (iter == args.end()) {
      const option_info& info = get_info(name);
      DEBUG("Using default %s = %s", name.c_str(), info.default_val);
      if (info.default_val == NULL) {
        return "";
      }
      return info.default_val;
    } else {
      DEBUG("Using provided %s = %s", name.c_str(), iter->second.c_str());
      return iter->second.c_str();
    }
  }
}

CmdlineOptions::CmdlineOptions(int argc, char* argv[])
  : app_name(argv[0]) {
  char c;
  int option_index = 0;
  while ((c = getopt_long(argc, argv, "hvld:f", options, &option_index)) != -1) {
    switch (c) {
      case 0: // long-form flag
        args[std::string(options[option_index].name)] =
          (optarg != NULL) ? std::string(optarg) : std::string();
        break;
      case '?': // unknown option, or missing required arg for option
        help_and_exit();
        break;
      default: // single-char flag
        args[std::string(get_option(c).name)] =
          (optarg != NULL) ? std::string(optarg) : std::string();
        break;
    }
  }
  if (optind < argc) {
    std::ostringstream oss;
    oss << "Unrecognized values:";
    for (int i = optind; i < argc; ++i) {
      oss << " " << argv[i];
    }
    ERROR(oss.str().c_str());
    help_and_exit();
  }

  // handle --help and --verbose internally:
  if (args.find(HELP) != args.end()) {
    help_and_exit();
  }
  if (args.find(VERBOSE) != args.end()) {
    config::debug_enabled = true;
  }
}


bool CmdlineOptions::list_devices() const {
  return parse_b(LIST_DEVICES);
}
std::string CmdlineOptions::device() const {
  return parse_s(DEVICE);
}

size_t CmdlineOptions::audio_collect_rate_hz() const {
  return parse_u(AUDIO_COLLECT_RATE, 1);
}
size_t CmdlineOptions::audio_sample_rate_hz() const {
  return parse_u(AUDIO_SAMPLE_RATE, 1);
}

size_t CmdlineOptions::display_fps_max() const {
  return parse_u(MAX_FPS, 1);
}
bool CmdlineOptions::display_vsync() const {
  return parse_b(VSYNC);
}
bool CmdlineOptions::display_fullscreen() const {
  return parse_b(FULLSCREEN);
}

size_t CmdlineOptions::bucket_count() const {
  return parse_u(BUCKET_COUNT, 1);
}
size_t CmdlineOptions::bucket_bass_exaggeration() const {
  size_t max = 900;
  return parse_u(BUCKET_BASS_EXAGGERATION, 0, &max);
}

size_t CmdlineOptions::color_lum_exaggeration() const {
  size_t max = 100;
  return parse_u(COLOR_LUM_EXAGGERATION, 0, &max);
}
size_t CmdlineOptions::color_max_lum() const {
  return parse_u(COLOR_MAX_LUM, 0);
}

size_t CmdlineOptions::analyzer_width_pct() const {
  size_t max = 100;
  return parse_u(ANALYZER_WIDTH_PCT, 0, &max);
}
size_t CmdlineOptions::voiceprint_scroll_rate() const {
  return parse_u(VOICEPRINT_SCROLL_RATE, 1);
}
size_t CmdlineOptions::loudness_adjust_rate() const {
  return parse_u(LOUDNESS_ADJUST_RATE, 0);
}


void CmdlineOptions::help_and_exit() const {
  fprintf(stderr, "usage: %s [args]\n\n", app_name.c_str());

  fprintf(stderr, "examples:\n");
  fprintf(stderr, "  %s\n", app_name.c_str());
  fprintf(stderr, "  %s --" FULLSCREEN "\n", app_name.c_str());
  fprintf(stderr, "  %s --" AUDIO_SAMPLE_RATE "=44100 --" DEVICE "=\"Sound Blaster 16\"\n\n", app_name.c_str());

  fprintf(stderr, "options:\n");
  for (size_t i = 0; ; ++i) {
    const struct option& opt = options[i];
    const struct option_info& info = option_infos[i];
    if (info.help_msg == NULL) {
      //stub option
      break;
    }

    if (opt.val == 0) {
      // no short arg
      fprintf(stderr, "  --%s", opt.name);
    } else {
      // with short arg
      fprintf(stderr, "  -%c / --%s", opt.val, opt.name);
    }
    if (info.default_val != NULL) {
      fprintf(stderr, " (=%s)", info.default_val);
    }
    fprintf(stderr, "\n    %s\n", info.help_msg);
  }

  fprintf(stderr, "\nkeys:\n");
  fprintf(stderr, "  Esc / Q / Ctrl+C / Alt+F4:\n    Exit\n");
  fprintf(stderr, "  R / Space:\n    Rotate display\n");
  fprintf(stderr, "  D:\n    Retrigger audio device autoselection\n");

  exit(-1);
}


bool CmdlineOptions::parse_b(const std::string& name) const {
  option_info info = get_info(name);
  arg_iter_t iter = args.find(name);
  const char* val;
  if (iter == args.end()) {
    DEBUG("Using default %s = %s", name.c_str(), info.default_val);
    val = info.default_val;
    if (val == NULL) {
      DEBUG("NULL default => false");
      return false;
    }
  } else if (iter->second.empty()) {
    DEBUG("Assuming %s = true", name.c_str());
    return true; // no value specified: assume enable
  } else {
    DEBUG("Using provided %s = %s", name.c_str(), iter->second.c_str());
    val = iter->second.c_str();
  }
  switch (val[0]) {
    case 'Y':
    case 'y':
    case 'T':
    case 't':
    case '1':
      DEBUG("Parsed %s = true", name.c_str());
      return true;
    case 'N':
    case 'n':
    case 'F':
    case 'f':
    case '0':
      DEBUG("Parsed %s = false", name.c_str());
      return false;
    default:
      ERROR("Unrecognized value: %s = %s (expected boolean)", name.c_str(), val);
      help_and_exit();
      return false; // happy compiler
  }
}

std::string CmdlineOptions::parse_s(const std::string& name) const {
  LOG("parse str %s", name.c_str());
  return pick_val(args, name);
}

size_t CmdlineOptions::parse_u(const std::string& name, size_t min, size_t* max_opt/*=NULL*/) const {
  LOG("parse size %s", name.c_str());
  const char* val = pick_val(args, name);
  char* first_invalid = NULL;
  size_t ret = strtoul(val, &first_invalid, 10);
  if (first_invalid != NULL && *first_invalid != '\0') {
    ERROR("Unrecognized value: %s = %s (offset %lu, expected unsigned int)",
        name.c_str(), val, first_invalid - val);
    help_and_exit();
    return 0; // happy compiler
  }
  DEBUG("Parsed %s = %lu", name.c_str(), ret);
  size_t max = (max_opt != NULL) ? *max_opt : std::numeric_limits<size_t>::max();
  if (ret < min || ret > max) {
    if (max_opt != NULL) {
      ERROR("Value must be within range [%lu,%lu]: %s = %s", min, max, name.c_str(), val);
    } else {
      ERROR("Value must be >= %lu: %s = %s", min, name.c_str(), val);
    }
    help_and_exit();
  }
  return ret;
}

double CmdlineOptions::parse_d(const std::string& name) const {
  LOG("parse dbl %s", name.c_str());
  const char* val = pick_val(args, name);
  char* first_invalid = NULL;
  double ret = strtod(val, &first_invalid);
  if (first_invalid != NULL && *first_invalid != '\0') {
    ERROR("Unrecognized value: %s = %s (offset %lu, expected double)",
        name.c_str(), val, first_invalid - val);
    help_and_exit();
    return 0; // happy compiler
  }
  DEBUG("Parsed %s = %.02f", name.c_str(), ret);
  if (ret < 0) {
    // all double values must be non-negative, at least for now.
    ERROR("Value must be non-negative: %s = %s", name.c_str(), val);
    help_and_exit();
  }
  return ret;
}
