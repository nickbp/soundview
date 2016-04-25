# SoundView

Simple voiceprint and analyzer for your computer.
Shows what's playing/recording with a high definition analyzer and voiceprint.

Licence: GPLv3

## Build and Run

The code is standard C++11 with the following dependencies:
* [CMake](https://cmake.org/) for the build system
* [SFML](http://www.sfml-dev.org/) (specifically SFML-Audio and SFML-Graphics) for interacting with system hardware
* [fftw](http://www.fftw.org/) for FFT of the audio stream

All of these tools are built with cross-platform support in mind, and therefore SoundView should more or less inherit that support.

In theory at least.

### Debian and Ubuntu

Here's the steps to build on an `apt`-based system, with your choice of compiler:

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

Here's the steps to build on a `yum`-based system, with your choice of compiler:

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

These steps assume a Win64 build. These are just what worked for me at the time, on a system running Windows 8.1. YMMV.

#### Downloads

- [7zip](http://www.7-zip.org/download.html) since fftw comes in `.tar.gz` format
- [Visual Studio](https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx) (14 aka 2015)
- [git](https://git-scm.com/downloads) (or use git included in VS?)
- [CMake](https://cmake.org/download/) (3.5.2)
- [SFML](http://www.sfml-dev.org/download.php) (2.3.2: "Visual C++ 14 (2015) - 64-bit")
- [FFTW binaries](http://www.fftw.org/install/windows.html) (3.3.4-dll64)
- [FFTW headers/source](http://www.fftw.org/download.html) (3.3.4)

#### Approx Build Steps

1. Grab the `soundview` repo with `git clone`.
2. Generate `.lib` files for FFTW:
  - Do the `lib /def` stuff mentioned in [the FFTW instructions](http://www.fftw.org/install/windows.html) using 'Developer Command Prompt'.
  - For 64bit, include an additional `/machine:x64` flag to the specified `lib` command.
3. Set up CMake using `cmake-gui.exe`:
  1. Set source dir to root-level `soundview/`, binary dir to whatever.
  2. Hit `Configure`. In the window that pops up, select `Visual Studio 14 2015 Win64` (or whatever's appropriate for you).
  3. After waiting for config to finish, CMake's `-NOTFOUND` variables need to be configured:
    - `fftw_INCLUDE_DIR` = `fftw-<ver>/api/` (unpacked from the FFTW `.tar.gz` with headers/source)
    - `fftw_LIBRARY` = `fftw-<ver>-dll64/libfftw3-3.lib` (unpacked and generated from the FFTW `.tar.gz` with binaries)
    - `sfml_INCLUDE_DIR` = `SFML-<ver>-windows-vc14-64-bit/SFML-<ver>/include/`
    - `sfml_*_LIBRARY` = matching `SFML-<ver>-windows-vc14-64-bit/SFML-<ver>/lib/sfml-*.lib` (ignore any `-d`/`-s`/`-s-d` versions)
  4. Hit `Configure` again and observe that the red rows get cleared.
  5. Hit `Generate` to create `<binary dir>/soundview.sln`.
4. Open Visual Studio and point it to the generated `<binary dir>/soundview.sln` created above.
5. Create a **RELEASE** build for the `soundview` project. There will be a lot of warnings since by default this project configures VS to be very pedantic.
6. Browse to `<binary dir>/Release/apps/`, which should contain `soundview.exe`. Copy the following libraries into `<binary dir>/Release/apps/`:
  - `<binary dir>/soundview/Release/soundview.dll`
  - `fftw-<ver>-dll64/libfftw3-3.dll`
  - `SFML-<ver>-windows-vc14-64-bit/SFML-<ver>/bin/`: `sfml-audio-2.dll`, `sfml-graphics-2.dll`, `sfml-system-2.dll`, and `sfml-window-2.dll`

With the `.dll`'s copied into the same directory as `soundview.exe`, you should now be able to run it. It can also be run via the console to see what audio devices it's finding.

If you're just seeing slowly extending vertical streaks, then it means SoundView effectively isn't getting any audio. Check the console output to see what devices are being found, and how much audio they're producing. If devices are producing less than a million "amplitude units" in the console output log, they're basically not producing audio.

The root problem is that Windows doesn't make it easy to tap into whatever audio is currently playing on the system. Your options are to either just display external audio that's brought in via the microphone jack, or to install software that creates a loopback audio device, like [this](http://software.muzychenko.net/eng/vac.htm#download) or [this](http://www.nerds.de/en/loopbeaudio.html), to create a recordable device that mirrors what is currently playing on the system.

### OSX

1. clone git repo
2. install xcode
3. get cmake for osx
4. download and install FFTW with homebrew:
   - `brew install fftw`
5. download and install SFML with homebrew:
   - `brew install sfml`
6. start cmake, point it to srcdir `soundview/`, builddir wherever
   - cmake should automatically find the correct locations for FFTW and SFML. If not, point it to wherever you have Homebrew set up to install them.
7. start xcode, open `<builddir>/soundview.xcodeproj`
8. run the build in xcode, success! now you just need to turn this into a usable binary. good luck! the problem is that soundview tries to create the window in a worker thread, which doesn't work on OS X (all GUI stuff is required to be done on the main thread in OS X). SFML detects this and refuses to even try, and because the program doesn't have a window it immediately exits.

## Usage

Keyboard shortcuts, features, and options are exposed through the help displayed by `./soundview -h`. Here's some additional information on configuring some of those options.

Note: Commandline arguments are currently unavailable on Windows, since Windows didn't have `getopt.h` and I'm not about to pull in another dependency just so that Windows has arg handling.

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
