# SoundView

Simple voiceprint and analyzer for your computer.
Shows what's playing/recording with a high definition analyzer and voiceprint.

Licence: GPLv3

## Build and Run

The code is standard C++11 with the following dependencies:
* [CMake](https://cmake.org/) for the build system
* [SFML](http://www.sfml-dev.org/) (specifically SFML-Audio and SFML-Graphics) for interacting with system hardware
* [fftw](http://www.fftw.org/) for FFT of the audio stream

All of these tools are built with cross-platform support in mind, and therefore SoundView should inherit that property (fingers crossed).

### Debian and Ubuntu

#### GCC

```sh
sudo apt-get update
sudo apt-get install build-essential cmake git libsfml-dev libfftw-dev

git clone git@github.com:nickbp/soundview.git

cd soundview; mkdir -p bin; cd bin
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4

cd apps
./soundview -h
```

#### Clang

```sh
sudo apt-get update
sudo apt-get install clang-3.8 cmake git libsfml-dev libfftw-dev

git clone git@github.com:nickbp/soundview.git

cd soundview; mkdir -p bin; cd bin
CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4

cd apps
./soundview -h
```

### Fedora and Red Hat

#### GCC

```sh
sudo yum update
sudo yum install gcc-c++ cmake git SFML-devel fftw-devel

git clone git@github.com:nickbp/soundview.git

cd soundview; mkdir -p bin; cd bin
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4

cd apps
./soundview -h
```

#### Clang

```
sudo yum update
sudo yum install clang cmake git SFML-devel fftw-devel

git clone git@github.com:nickbp/soundview.git

cd soundview; mkdir -p bin; cd bin
CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4

cd apps
./soundview -h
```

### Windows

TODO

### OSX

TODO

## Usage

Keyboard shortcuts, features, and options are exposed through the help displayed by `./soundview -h`. Here's some additional information on configuring some of those options.

### Device Selection

By default, soundview automatically selects the device to display by selecting the device that produce the most audio.
Once soundview is running, this autodetection may be retriggered by pressing `D`.

``` sh
./soundview # autodetect device at startup
```

This autoselection may be overridden by manually specifying a device by id or by name.

```
./soundview -l # list devices (or see printed list when autodetection is occurring)
./soundview -d 3 # use device 3 in the list
./soundview -d "Sound Blaster 16" # use device with this name
```

### Display Options

There are multiple features in play for adjusting the appearance of the visualization. Each may be customized via commandline arguments.

- `-f/--fullscreen` Start the display in fullscreen mode.
- `--voiceprint-scroll-rate` (px) How much to shift the voiceprint for each rendered frame.
- `--loudness-adjust-rate` (0-inf) The charted data is self-adjusting relative to the loudness of the audio stream. This determines how quickly to increase sensitivity during quiet periods.
- `--bucket-bass-exaggeration` (0-900) This value may be increased or decreased to adjust the amount of scaling that's given to bass/mids. By default, bass values are given more width in the display than they would otherwise. This makes bass/mids easier to see, otherwise they're very small relative to higher pitches.
- `--color-lum-exaggeration` (0-100) This setting determines how much to brighten quiet values. Quieter values are difficult to see without some exaggeration.
- `--color-max-lum` (0-inf) The maximum luminosity value to use when coloring louder values. Adjusting this changes how colors are displayed.
- `--analyzer-width-pct` (%) How much of the display should be taken up by the spectrum analyzer. Setting this to 0 results in only rendering the voiceprint, while 100 results in only rendering the analyzer.

### Performance Options

The display starts at a fairly high definition which can be adjusted up or down via commandline arguments. In particular, the following can be adjusted to increase or decrease the display quality, with proportional changes to system load.

- `--bucket-count` (#) This is the number of columns to be displayed in the spectrum. This is likely the single flag that's most relevant to performance, and it's tied to `--audio-sample-rate` in that more columns require more data.
- `--audio-sample-rate` (Hz) The rate of the stream to read from the audio device. If this is turned too low, the display will tend to refresh at a slower rate since it will be starved for audio data.
- `--audio-collect-rate` (Hz) How frequently the audio device should be polled for data. Ideally this should be at or above the display refresh rate, but it shouldn't otherwise have too much impact on performance.
- `--max-fps` (Hz) The frames per second to display at. This should be set to the display refresh rate (usually 60, the default), going beyond this just wastes CPU.
