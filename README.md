# 8Beat

![GitHub License](https://img.shields.io/github/license/razterizer/8Beat?color=blue)
![Static Badge](https://img.shields.io/badge/linkage-header_only-yellow)
![Static Badge](https://img.shields.io/badge/3rdparty_linkage-static_/_dynamic-yellow)
![Static Badge](https://img.shields.io/badge/C%2B%2B-20-yellow)

[![build and test ubuntu](https://github.com/razterizer/8Beat/actions/workflows/build-and-test-ubuntu.yml/badge.svg)](https://github.com/razterizer/8Beat/actions/workflows/build-and-test-ubuntu.yml)
[![build macos](https://github.com/razterizer/8Beat/actions/workflows/build-macos.yml/badge.svg)](https://github.com/razterizer/8Beat/actions/workflows/build-macos.yml)
[![build ubuntu](https://github.com/razterizer/8Beat/actions/workflows/build-windows.yml/badge.svg)](https://github.com/razterizer/8Beat/actions/workflows/build-windows.yml)

![Top Languages](https://img.shields.io/github/languages/top/razterizer/8Beat)
![GitHub repo size](https://img.shields.io/github/repo-size/razterizer/8Beat)
![C++ LOC](https://raw.githubusercontent.com/razterizer/8Beat/badges/loc-badge.svg)
![Commit Activity](https://img.shields.io/github/commit-activity/t/razterizer/8Beat)
![Last Commit](https://img.shields.io/github/last-commit/razterizer/8Beat?color=blue)
![Contributors](https://img.shields.io/github/contributors/razterizer/8Beat?color=blue)

This library is a cross-platform and header only library (except for 3rd-party libs necessary).
This library is for synthesizing, loading and saving sound and also has a chip-tune engine which allow you full flexibility to design instruments and to use these instruments when composing your chip-tune tune.
The library depends on an implementation of the interface `AudioLibSwitcher`. See below for more info.

Documentation is a work in progress for the moment being. For example on how to use this lib, please clone repo [`Pilot Episode`](https://github.com/razterizer/Pilot-Episode) and follow the instructions there.

## Licensing

### LibSndFile

`WaveformIO.h` uses [`libsndfile`](https://github.com/libsndfile/libsndfile?tab=LGPL-2.1-1-ov-file) which is under the LGPL-2.1 license. There is no derivative work of the library whatsoever. `libsndfile` is only a dependency. 
In order to be able to use `WaveformIO.h` you need to supply the libsnd binaries (libs or dlls) and headers yourself. These are not included.

Since `libsndfile` is under LGPL, this means that whenever a release build is created, its source code has to be included as well. So that is important to remember.

See this online discussion for more info: https://softwareengineering.stackexchange.com/questions/141847/how-does-using-a-lgpl-gem-affect-my-mit-licensed-application

### OpenAL Soft

This library (which is under the MIT license) heavily relies on the OpenAL Soft license which is under the LGPL license (see https://www.openal-soft.org/).

Here OpenAL Soft is also only used as a dependency and `8Beat` is not to be considered as derived work. There are no binaries (libs or dlls), nor headers from OpenAL Soft included in this repository. The user has to supply those him/her-self.

One issue however is that the newer OpenAL Soft repository on github (https://github.com/kcat/openal-soft) seems to be under GPL-v2 license. This could be a potential issue!!!

Edit: See this discussion with the author of the lib: https://github.com/kcat/openal-soft/issues/187:
```
kcat:

Hi.

Generally speaking, as long as OpenAL Soft is unmodified and distributed as a shared library, you can use it in any project you wish. Unless your code is also (L)GPL, that's typically the easiest way to deal with LGPL libraries. Otherwise, if OpenAL Soft itself is static-linked into a binary, then any distribution must also offer the source or object files of the binary so a user can relink with another version of OpenAL Soft.
```

### applaudio

[`applaudio`](https://github.com/razterizer/applaudio) is an MIT option to OpenAL. It doesn't have nearly as many features as OpenAL has, but it has enough to be useful for simple terminal-based games and such. More features and also possibly more backends will be added in the future. It implements the [`IAudioLibSwitcher`](https://github.com/razterizer/AudioLibSwitcher) interface which makes it easy to switch out OpenAL to applaudio as long as you're fine with that subset of features of that interface. The excellent thing with an MIT-based audio library is that we/you now finally can build releases and post on the github project pages where appropriate.

Applaudio needs more testing and probably a bit more development to mature, but I think this is a great start.
Also, applaudio is header-only (except for the backends: ALSA and CoreAudio. WASAPI uses ole32.lib, and you don't need to worry about that one as it is a system lib).

## Definitions

We'll often use the term `waveform` as meaning an audio signal here.

## The Library

8Beat is a header-only library that contains the following header files:

* `Waveform.h` <br/> contains the struct `Waveform` that contains an audio buffer, and variables `frequency`, `sample_rate` and `duration`. This struct is very central as it holds the PCM audio waveform representation itself.

* `WaveformIO.h` <br/> contains class `WaveformIO` that has two static public functions: `load()` and `save()`. These functions rely on the [`sndfile`](https://github.com/libsndfile/libsndfile) library which allows you to import and export a `Waveform` object to many different types and formats.
* `WaveformGeneration.h` <br/> contains class `WaveformGeneration` with a single public function `generate_waveform()`.
  * You can control the created waveform via these optional parameters in struct `WaveformGenerationParams`:
    ```C++
    std::optional<float> sample_range_min = std::nullopt; // default: -1
    std::optional<float> sample_range_max = std::nullopt; // default: +1
    // duty_cycle applies to SQUARE, TRIANGLE and SAWTOOTH.
    // In the case of TRIANGLE, duty_cycle = 1 is a SAWTOOTH and 0 is reverse SAWTOOTH.
    // In the case of SAWTOOTH, when duty_cycle goes from 1 to 0, the SAWTOOTH teeth get thinner towards 0.
    // default: 0.5 for SQUARE and TRIANGLE and 1 for SAWTOOTH.
    std::optional<float> duty_cycle = std::nullopt;
    std::optional<float> duty_cycle_sweep = std::nullopt; // unit/s.
    std::optional<float> min_frequency_limit = std::nullopt;
    std::optional<float> max_frequency_limit = std::nullopt;
    std::optional<float> freq_slide_vel = std::nullopt; // 8va/s
    std::optional<float> freq_slide_acc = std::nullopt; // 8va/s^2
    std::optional<float> freq_vibrato_depth = std::nullopt;
    std::optional<float> freq_vibrato_freq = std::nullopt;
    std::optional<float> freq_vibrato_freq_vel = std::nullopt;
    std::optional<float> freq_vibrato_freq_acc = std::nullopt;
    std::optional<float> freq_vibrato_freq_acc_max_vel_limit = std::nullopt;
    std::optional<float> freq_vibrato_phase = std::nullopt;
    std::optional<float> vibrato_depth = std::nullopt;
    std::optional<float> vibrato_freq = std::nullopt;
    std::optional<float> vibrato_freq_vel = std::nullopt;
    std::optional<float> vibrato_freq_acc = std::nullopt;
    std::optional<float> vibrato_freq_acc_max_vel_limit = std::nullopt;
    std::optional<float> vibrato_phase = std::nullopt;
    int noise_filter_order = 2;
    float noise_filter_rel_bw = 0.2f;
    float noise_filter_slot_dur_s = 1e-2f;
    std::vector<ArpeggioPair> arpeggio;
    ```
* `Spectrum.h` <br/> contains struct `Spectrum` which is used in conjunction with functions such as public functions `fft()` and `ifft()` in class `WaveformHelper`.
* `WaveformHelper.h` <br/> contains class `WaveformHelper` which has the following public static functions:
  * `subset()` allows you to retrieve a portion of a waveform.
  * `mix()` mixes two waveforms by lerping them or multiple waveforms by weighted average.
  * `ring_modulation()` multiplies two waveforms.
  * `reverb()` does reverb between a waveform and an impulse response waveform of an environment (response sound from a dirac pulse-like "trigger" sound) to create a reverb effect.
  * `reverb_fast()` same as `reverb()` but is very fast because it uses the fast Fourier transform.
  * `complex2real()` lets you choose if you want the real part, imag part or absolute value of both from a given complex value.
  * `fft()` this is the fast Fourier transform using the Cooley-Tukey algorithm.
  * `ifft()` this is the fast inverse Fourier transform using a variant of the Cooley-Tukey algorithm.
  * `find_min_max()` finds the min and max values of a given audio signal.
  * `normalize_over()` only normalize if the amplitude is larger than a certain limit. If so then normalized to that limit. This is a kind of a normalized amplitude limiter.
  * `normalize()` normalizes the waveform so that the max amplitude is always (nearly) 1.
  * `normalize_scale()` same as `normalize()` but is followed by a scaling operation so that the max amlitude = scaling_factor.
  * `scale()` simply just scale the waveform with a scale factor.
  * `clamp()` clamps the samples of a waveform within a specified range.
  * `fir_moving_average()` a moving average filter of sorts.
  * `fir_sinc_window_low_pass()` a kind of a low-pass filter.
  * `flanger()` applies a flanger filter to the input waveform.
  * `karplus_strong()` generates guitar-like string sounds.
  * `envelope_adsr()` applies an adsr envelope to a specified waveform.
  * `resample()` resamples a waveform to a specified sample-rate.
  * `filter(const Waveform&, const FilterArgs&)` filters a waveform according to the `FilterArgs` argument. Calls the function signature below:
  * `filter(const Waveform& wave,
            FilterType type,
            FilterOpType op_type,
            int filter_order,
            float freq_cutoff_hz, std::optional<float> freq_bandwidth_hz,
            float ripple = 0.1f, // ripple: For Chebychev filters.
            bool normalize_filtered_wave = false)` where:
    * `type` is `NONE`, `Butterworth`, `ChebyshevTypeI` or `ChebyshevTypeII`.
    * `op_type` is `NONE`, `LowPass`, `HighPass`, `BandPass` or `BandStop`.
  * `filter(const Waveform&, const Filter&)` filters a general FIR or IIR filter with coeffs `a` and `b`. Used internally by above filter functions.
  * `filter(const std::vector<float>&, const Filter&)` used by the function in the previous point.
  * `print_waveform_graph_idx()` and `print_waveform_graph_t()` prints the waveform shape in the terminal.
  * `calc_time_from_num_cycles()` utility function for waveform objects.
  * `calc_dt()` utility function for waveform objects.
  * `calc_duration()` utility function for waveform objects.
  * `calc_num_samples()` utility function for waveform objects.
* `ADSR.h` <br/> contains data structures to represent various aspects of an ADSR envelope. Also contains the namespace `adsr_presets` with a number of ADSR presets that will give you an `ADSR` struct.
* `Synthesizer.h` <br/> contains class `Synthesizer` which allows you to produce a synthesized instrument sound via the static public functions `synthesize()`. These are the supported instruments at the moment:
  * `PIANO` (Not really piano-like, but it's a start).
  * `VIOLIN` (Can perhaps serve as a violin-ish sound if you squint with your ears).
  * `ORGAN`.
  * `TRUMPET` (Not quite there yet, but not too shabby though).
  * `FLUTE` (Sounds kind of flute-ish).
  * `GUITAR` (Need to rework the Karplus-Strong algorithm to make it sound as it's supposed to).
  * `KICKDRUM`.
  * `SNAREDRUM`.
  * `HIHAT`.
  * `ANVIL` (Well, it kind of sounds like an anvil doesn't it?).
* `AudioSourceHandler.h` <br/> contains classes `AudioSourceHandler`, `AudioSource` and `AudioStreamSource`. `AudioSourceHandler` produces instances of `AudioSource` and `AudioStreamSource`.
* `ChipTuneEngine.h` <br/> contains class `ChipTuneEngine` which allows you to play a chiptune from a text-file (file ending `*.ct`) in a threaded manner so that you can use it in games and what-not. In the beginning of the tune file you define the instruments, adsr envelopes, low-pass filters etc. Then after that you define the score where each column is a voice or channel if you will, and each column is a beat (bars are made up of beats, you could say). Refer to [this wiki page](https://github.com/razterizer/8Beat/wiki/ChipTuneEngine-Format) about the file format.


# Getting Started

This header-only library depends on the following set of header-only libraries if you intend to use it with OpenAL Soft:
* [`Core`](https://github.com/razterizer/Core)
* [`TrainOfThrought`](https://github.com/razterizer/TrainOfThought)
* [`AudioLibSwitcher_OpenAL`](https://github.com/razterizer/AudioLibSwitcher_OpenAL)
* Windows: [`3rdparty_OpenAL`](https://github.com/razterizer/3rdparty_OpenAL)
* Windows: [`3rdparty_libsndfile`](https://github.com/razterizer/3rdparty_libsndfile) (only necessary for `WaveformIO.h`)

or this set of header-only libraries if you intend to use it with applaudio (now by default):
* [`Core`](https://github.com/razterizer/Core)
* [`TrainOfThrought`](https://github.com/razterizer/TrainOfThought)
* [`AudioLibSwitcher_applaudio`](https://github.com/razterizer/AudioLibSwitcher_applaudio)
* [`applaudio`](https://github.com/razterizer/applaudio)
* Windows: [`3rdparty_libsndfile`](https://github.com/razterizer/3rdparty_libsndfile) (only necessary for `WaveformIO.h`)

and uses the header-only library [`Termin8or`](https://github.com/razterizer/Termin8or) only for the demos in either case.

These libs are expected to be located in checkout dirs with the same names and next to each other. Like this for OpenAL Soft:
```xml
<my_source_code_dir>/lib/Core/
<my_source_code_dir>/lib/TrainOfThought/              ; Used by `WaveformHelper.h`.
<my_source_code_dir>/lib/AudioLibSwitcher_OpenAL/     ; Allows you to choose between applaudio and OpenAL Soft. Minimal common API.
<my_source_code_dir>/lib/3rdparty_OpenAL/             ; Windows only. The OpenAL-Soft libs/dlls necessary for the **Windows** build.
<my_source_code_dir>/lib/3rdparty_libsndfile/         ; Windows only. The libsndfile libs/dlls necessary for `WaveformIO.h` to work on windows.
<my_source_code_dir>/lib/Termin8or/                   ; Only for the demos.
<my_source_code_dir>/lib/8Beat/
```
or like this for applaudio:
```xml
<my_source_code_dir>/lib/Core/
<my_source_code_dir>/lib/TrainOfThought/              ; Used by `WaveformHelper.h`.
<my_source_code_dir>/lib/AudioLibSwitcher_applaudio/  ; Allows you to choose between applaudio and OpenAL Soft. Minimal common API.
<my_source_code_dir>/lib/applaudio/                   ; Header-only audio library. No libs/dlls except for the ones from the backends.
<my_source_code_dir>/lib/3rdparty_libsndfile/         ; Windows only. The libsndfile libs/dlls necessary for `WaveformIO.h` to work on windows.
<my_source_code_dir>/lib/Termin8or/                   ; Only for the demos.
<my_source_code_dir>/lib/8Beat/
```
where `<my_source_code_dir>` is where you normally put your git repos and `<lib>` is recommended to be "`lib`" but can be named something different or left out all-together. However, the following programs requires them to be located in a sub-folder called "`lib`" or else these programs will not build (unless you change the header search paths for the xcode/vcxproj projects):
* [`Pilot_Episode`](https://github.com/razterizer/Pilot_Episode)
* [`Christmas_Demo`](https://github.com/razterizer/Christmas_Demo)
* [`Asciiroids`](https://github.com/razterizer/Asciiroids)

There are currently ten demos under the `demos` folder that you can build and run under debian-based / macos / windows.
First cd to folder `demos`. To build `demo_1` type `./build_demo_1.sh`. Then run by typing `./bin/demo_1`. The same applies for the other demos. You can build all demos by running the script `./build_all_demos.sh` (debian-based / macos) or `build_all_demos.bat` (windows).

An easier way to get started is to use any of the scripts `setup_and_build_demos_debian.sh`, `setup_and_build_demos_macos.sh` or `setup_and_build_demos.bat`. If you use these, you can then skip the section below. These scripts automatically clones the repos necessary for your platform.

## 3rd-party Libraries

You'll need [`OpenAL Soft`](https://www.openal-soft.org) or [`applaudio`](https://github.com/razterizer/applaudio) for all demos and [`sndfile`](https://github.com/libsndfile/libsndfile) for some of the demos.

However, the libs and dlls from these are mirrored in [`3rdparty_OpenAL`](https://github.com/razterizer/3rdparty_OpenAL) and [`3rdparty_libsndfile`](https://github.com/razterizer/3rdparty_libsndfile) respectively (nothing like this necessary for using applaudio) and `8Beat` expects the folder structure used in these repos, so the most easy way to get started is to use any of the `setup_and_build_demos` scripts in the root folder. See the `Getting Started` section for the folder layout that is expected by `8Beat`.

### Windows:

On Windows no installation of **OpenAL Soft** or **libsndfile** is necessary. Only the `3rdparty_OpenAL` (and optionally `3rdparty_libsndfile`) repos are necessary. See above.

If you use applaudio (by default now) you don't even need any `3rdparty_whatever` repo. See section `AudioLibSwitcher` below for more info. on how to switch between OpenAL and applaudio.

### MacOS:

Install **OpenAL Soft** using `brew install openal-soft`. The build script(s) takes care of the rest (adjust script if paths differ).

If using applaudio instead of OpenAL Soft, then no installation is required.

Install **libsndfile** using `brew install libsndfile`. The build script(s) takes care of the rest (adjust script if paths differ).

### Linux (Ubuntu):

Install **OpenAL Soft** using `sudo apt install libopenal-dev`.

If using applaudio instead of OpenAL Soft, then no installation is required.

Install **libsndfile** using `sudo apt install libsndfile-dev`.

## AudioLibSwitcher : Switching from OpenAL Soft to applaudio for your program

This documentation here can be found in [`AudiLibSwitcher`](https://github.com/razterizer/AudioLibSwitcher) as well.

This section describes how to change your application code to work with either `applaudio` or `OpenAL Soft`.

As my own audio library [`applaudio`](https://github.com/razterizer/applaudio) is becoming more and more stable, you have the option to choose between OpenAL/OpenAL_Soft (GPL-based) and applaudio (MIT). This is nice, because it will allow you to build a release that is entirely MIT-licensed without having to be infected by the GPL-virus.

### 8Beat

To make 8Beat work with applaudio then make sure that the define `USE_APPLAUDIO` is defined in `AudioSourceHandler.h` in the 8Beat repo. If you want to use OpenAL Soft instead, then undefine it by commenting out the define.

### Dependencies

The `dependencies` file of your application repo can now look like this:
```
lib/Core                       https://github.com/razterizer/Core.git                   dbe2f701a255d578308c254839a3641786777658
lib/Termin8or                  https://github.com/razterizer/Termin8or.git              fb8e4ce8efabe83167192c5d82c4448e6ec8b45f
lib/8Beat                      https://github.com/razterizer/8Beat                      041761531cdc6721d4aea07da63c0b2a5b7403d7
lib/AudioLibSwitcher_applaudio https://github.com/razterizer/AudioLibSwitcher_applaudio db2648c00533a8894339095b0e727989e3ae7425
# lib/AudioLibSwitcher_OpenAL    https://github.com/razterizer/AudioLibSwitcher_OpenAL    811c60c23a446d5f2894e9379c938df19f889c41
lib/TrainOfThought             https://github.com/razterizer/TrainOfThought             fe8a5f0fd7e492bb1e8dfffac9aef1ee888735da
lib/applaudio                  https://github.com/razterizer/applaudio                  702d425e551f19717baed605ae7983ceddb1587b
# lib/3rdparty_OpenAL            https://github.com/razterizer/3rdparty_OpenAL            d8361648d7b505154109f1ba074922555a96e5de                    win
```
This way you'll using locked and stable versions of each library. Things relating to `OpenAL` is now commented out here.

### XCode Project

On Mac in the XCode project you can now choose `OpenAL` if you still want to use that:
<img width="1089" height="441" alt="image" src="https://github.com/user-attachments/assets/7f62fc4a-8575-440b-8a47-e2163543c1a1" />

or use `AudioToolbox`, `CoreAudio` and `CoreFoundation` if you want to use `applaudio` instead:
<img width="1074" height="409" alt="image" src="https://github.com/user-attachments/assets/b409d98d-dded-477c-88a9-58b789b4501e" />

In your XCode project you may have the following search paths. For example:
<img width="935" height="343" alt="image" src="https://github.com/user-attachments/assets/92b9bcae-4164-4312-ae38-a9c645e2b423" />

If using `OpenAL` then you can keep `../lib/AudioLibSwitcher_OpenAL/include/` and `/opt/homebrew/opt/openal-soft/include/`, but if you use `applaudio` you can just keep `../lib/AudioLibSwitcher_applaudio/include/` and `../lib/applaudio/include/` instead.

You also need to define `USE_APPLAUDIO` for both the `Release` and the `Debug` targets. Like this:
<img width="620" height="129" alt="image" src="https://github.com/user-attachments/assets/78df78b6-1952-434a-8303-d2defceeb08f" />


### Build Script

Then your build script for MacOS (CoreAudio) and Linux (here Debian-based) (ALSA) would be something like:
```bash
#!/bin/bash

os_name=$(uname)

if [[ $os_name == *"Darwin"* ]]; then
  additional_flags="\
    -I../../lib/Core/include \
    -I../../lib/Termin8or/include \
    -I../../lib/8Beat/include \
    -I../../lib/TrainOfThought/include \
    -I../../lib/AudioLibSwitcher_applaudio/include \
    -I../../lib/applaudio/include -framework AudioToolbox -framework CoreAudio -framework CoreFoundation -DUSE_APPLAUDIO"
  # -I../../lib/AudioLibSwitcher_OpenAL/include \
  # -I/opt/homebrew/opt/openal-soft/include -L/opt/homebrew/opt/openal-soft/lib -lopenal"
else
  additional_flags="\
    -I../../lib/Core/include \
    -I../../lib/Termin8or/include \
    -I../../lib/8Beat/include \
    -I../../lib/TrainOfThought/include \
    -I../../lib/AudioLibSwitcher_applaudio/include \
    -I../../lib/applaudio/include -DUSE_APPLAUDIO"
  # -I../../lib/AudioLibSwitcher_OpenAL/include"
  #export BUILD_PKG_CONFIG_MODULES='openal'
  export BUILD_PKG_CONFIG_MODULES='alsa'
fi

../../lib/Core/build.sh asciiroids "$1" "${additional_flags[@]}"

# Capture the exit code of Core/build.sh
exit_code=$?

if [ $exit_code -ne 0 ]; then
  echo "Core/build.sh failed with exit code $exit_code"
  exit $exit_code
fi

### Post-Build Actions ###

mkdir -p bin/fonts/
cp ../../lib/Termin8or/include/Termin8or/title/fonts/* bin/fonts/

cp music.ct bin/
```

For MacOS, the important bits above are those concerning `AudioLibSwitcher_applaudio`, `applaudio`, `-framework AudioToolbox`, `-framework CoreAudio`, `-framework CoreFoundation` and `-DUSE_APPLAUDIO`.

For Linux, the important bits above are those concerning `AudioLibSwitcher_applaudio`, `applaudio`, `-DUSE_APPLAUDIO` and `export BUILD_PKG_CONFIG_MODULES='alsa'`.

### Windows vcxproj Changes

Then finally for the Windows build you need to change the external include paths from this (OpenAL-based):

```xml
<ExternalIncludePath>$(SolutionDir)\..\..\lib\Core\include;$(SolutionDir)\..\..\lib\Termin8or\include;$(SolutionDir)\..\..\lib\8Beat\include;$(SolutionDir)\..\..\lib\TrainOfThought\include;$(SolutionDir)\..\..\lib\AudioLibSwitcher_OpenAL\include;$(SolutionDir)\..\..\lib\3rdparty_OpenAL\include</ExternalIncludePath>
```

to this (applaudio-based)

```xml
<ExternalIncludePath>$(SolutionDir)\..\..\lib\Core\include;$(SolutionDir)\..\..\lib\Termin8or\include;$(SolutionDir)\..\..\lib\8Beat\include;$(SolutionDir)\..\..\lib\TrainOfThought\include;$(SolutionDir)\..\..\lib\AudioLibSwitcher_applaudio\include;$(SolutionDir)\..\..\lib\applaudio\include</ExternalIncludePath>
```

You also need to change the linkage from this (OpenAL-based):

```xml
<AdditionalDependencies>openal32.lib;%(AdditionalDependencies)</AdditionalDependencies>
<AdditionalLibraryDirectories>$(SolutionDir)\..\..\lib\3rdparty_OpenAL\lib;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
```

to this (applaudio-based):

```xml
<AdditionalDependencies>ole32.lib;%(AdditionalDependencies)</AdditionalDependencies>
```

And remove the copying of the OpenAL dll:

```xml
<PostBuildEvent>
      <Command>xcopy "$(SolutionDir)\music.ct" "$(TargetDir)" /Y
xcopy "$(SolutionDir)\..\..\lib\Termin8or\include\Termin8or\title\fonts\*" "$(TargetDir)\fonts\" /Y
xcopy "$(SolutionDir)..\..\lib\3rdparty_OpenAL\lib\OpenAL32.dll" "$(TargetDir)" /Y</Command>
    </PostBuildEvent>
```

so it becomes:

```xml
<PostBuildEvent>
      <Command>xcopy "$(SolutionDir)\music.ct" "$(TargetDir)" /Y
xcopy "$(SolutionDir)\..\..\lib\Termin8or\include\Termin8or\title\fonts\*" "$(TargetDir)\fonts\" /Y
    </PostBuildEvent>
```

Also to signal to the `8Beat/AudioSourceHandler.h` that you want to use the applaudio lib rather than OpenAL lib, then you need to add the `USE_APPLAUDIO` define:

```xml
<PreprocessorDefinitions>WIN32;_DEBUG;_CONSOLE;NOMINMAX;USE_APPLAUDIO;%(PreprocessorDefinitions)</PreprocessorDefinitions>
```
