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
![Goto](https://img.shields.io/github/search/razterizer/8Beat/goto)
![Commit Activity](https://img.shields.io/github/commit-activity/t/razterizer/8Beat)
![Last Commit](https://img.shields.io/github/last-commit/razterizer/8Beat?color=blue)
![Contributors](https://img.shields.io/github/contributors/razterizer/8Beat?color=blue)
<!-- ![Languages](https://img.shields.io/github/languages/count/razterizer/8Beat) -->

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
    std::optional<float> vibrato_depth = std::nullopt;
    std::optional<float> vibrato_freq = std::nullopt;
    std::optional<float> vibrato_freq_vel = std::nullopt;
    std::optional<float> vibrato_freq_acc = std::nullopt;
    std::optional<float> vibrato_freq_acc_max_vel_limit = std::nullopt;
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

This header-only library depends on the following header-only libraries:
* [`Core`](https://github.com/razterizer/Core)
* [`AudioLibSwitcher_OpenAL`](https://github.com/razterizer/AudioLibSwitcher_OpenAL)
* [`TrainOfThrought`](https://github.com/razterizer/TrainOfThought)

and uses the header-only library [`Termin8or`](https://github.com/razterizer/Termin8or) for the demos.

These libs are expected to be located in checkout dirs with the same names and next to each other. Like this:
```xml
<source>/<lib>/Core
<source>/<lib>/AudioLibSwitcher_OpenAL
<source>/<lib>/Termin8or
<source>/<lib>/TrainOfThought
```
where `<source>` is where you normally put your git repos and `<lib>` is recommended to be "`lib`" but can be named something different or left out all-together. The game [`Pilot_Episode`](https://github.com/razterizer/Pilot_Episode) however, requires them to be located in a sub-folder called "`lib`" or else the game will not build.

The header only libs uses relative include paths (which is mayhaps a bit suboptimal), but I'll see if I can find a better solution for this in the future.

There are currently eight demos under the `demos` folder that you can build and run under linux / macos.
First cd o folder `demos`. To build `demo_1` type `./build_demo_1.sh l`. Then run by typing `./bin_linux/demo_1`. The same applies for the other demos. You can build all demos by running the script `./build_all_demos.sh`.

## 3rd-party Libraries

You'll need [`OpenAL Soft`](https://www.openal-soft.org) for all demos and [`sndfile`](https://github.com/libsndfile/libsndfile) for some of the demos.

### Windows:

Grab a copy of [`OpenAL Soft`](https://www.openal-soft.org). Then copy its files to below folders according to the instructions.

You need the following 3rdparty folder with subfolders:
```xml
<my_source_code_dir>/lib/3rdparty/
<my_source_code_dir>/lib/3rdparty/include/
<my_source_code_dir>/lib/3rdparty/include/OpenAL_Soft/
<my_source_code_dir>/lib/3rdparty/lib/
```

`<my_source_code_dir>/lib/3rdparty/lib/` should contain:
* `OpenAL32.lib`.
* `sndfile.lib`.
* `sndfile.dll`.

The dll `sndfile.dll` should then be copied to where the executable lands.

`<my_source_code_dir>/lib/3rdparty/include/` should contain:
* `sndfile.h`.

`<my_source_code_dir>/lib/3rdparty/include/OpenAL_Soft/` should contain:
* `al.h`.
* `alc.h`.
* `alext.h`.
* `efx.h`.
* `efx-creative.h`.
* `efx-presets.h`.

### MacOS:

Install **OpenAL Soft** using `brew install openal-soft`. The build script(s) takes care of the rest (adjust script if paths differ).

Install **libsndfile** using `brew install libsndfile`. The build script(s) takes care of the rest (adjust script if paths differ).

### Linux (Ubuntu):

Install **OpenAL Soft** using `sudo apt install libopenal-dev`.

Install **libsndfile** using `sudo apt install libsndfile-dev`.
