# 8Beat

Documentation is a work in progress for the moment being. For example on how to use this lib, please clone repo [`Pilot Episode`](https://github.com/razterizer/Pilot-Episode) and follow the instructions there.

## Definitions

We'll often use the term `waveform` as meaning an audio signal here.

## The Library

8Beat is a header-only library that contains the following header files:

* `Waveform.h` <br/> contains the struct `Waveform` that contains an audio buffer, and variables `frequency`, `sample_rate` and `duration`. This struct is very central as it holds the PCM audio waveform representation itself.

* `WaveformIO.h` <br/> contains class `WaveformIO` that has two static public functions: `load()` and `save()`. These functions rely on the [`sndfile`](https://github.com/libsndfile/libsndfile) library which allows you to import and export a `Waveform` object to many different types and formats.
* `WaveformGeneration.h` <br/> contains class `WaveformGeneration` with a single public function `generate_waveform()`.
* `Spectrum.h` <br/> contains struct `Spectrum` which is used in conjunction with functions such as public functions `fft()` and `ifft()` in class `WaveformHelper`.
* `WaveformHelper.h` <br/> contains class `WaveformHelper` which has the following public static functions:
  * `mix()` mixes two waveforms by lerping them or multiple waveforms by weighted average.
  * `ring_modulation()` multiplies two waveforms.
  * `reverb()` does reverb between a waveform and an impulse response waveform of an environment (response sound from a dirac pulse-like "trigger" sound) to create a reverb effect.
  * `reverb_fast()` same as `reverb()` but is very fast because it uses the fast Fourier transform.
  * `complex2real()` lets you choose if you want the real part, imag part or absolute value of both from a given complex value.
  * `fft()` this is the fast Fourier transform using the Cooley-Tukey algorithm.
  * `ifft()` this is the fast inverse Fourier transform using a variant of the Cooley-Tukey algorithm.
  * `find_min_max()` finds the min and max values of a given audio signal.
  * `normalize_over()` only normalize if the amplitude is larger than a certain limit. If so then normalized to that limit. This is a kind of a normalized amplitude limiter.
  * `scale()` simply just scale the waveform with a scale factor.
  * `clamp()` clamps the samples of a waveform within a specified range.
  * `fir_moving_average()` a moving average filter of sorts.
  * `fir_sinc_window_low_pass()` a kind of a low-pass filter.
  * `karplus_strong()` generates guitar-like string sounds.
  * `envelope_adsr()` applies an adsr envelope to a specified waveform.
  * `resample()` resamples a waveform to a specified sample-rate.
  * `filter_low_pass()` filters a waveform using either a Butterworth, Chebyshev Type I or Chebyshev Type II low-pass filter.
  * `print_waveform_graph()` prints the waveform shape in the terminal.
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


## Getting Started

This header-only library depends on the header-only library Core-Lib which should be located in a checkout-folder called [`Core Lib`](https://github.com/razterizer/Core-Lib) next to the checkout-folder for this lib (which I recommend you call `8-Bit Audio Emulator Lib`). I intend to change these paths names in the near future. The header only libs uses relative include paths (which is a bit suboptimal), but I'll see if I can find a better solution for this.

There is currently one demo under the `demos` folder that you can build and run under linux / macos.
First cd o folder `demos`. To build type `./build_demo_1.sh l`. Then run by typing `./bin_linux/demo_1`.

### 3rd-party Libraries

You'll need OpenAL for all demos and [`sndfile`](https://github.com/libsndfile/libsndfile) for some of the demos.

* **OpenAL on Windows**: Grab a copy of [`OpenAL_Soft`](https://www.openal-soft.org) and setup linking for that.
* **OpenAL on MacOS**: Install openal using `brew install openal`. The build script(s) takes care of the rest (adjust script if paths differ).
* **OpenAL on Linux**: Use whatever package management system that is available on your distro.

* **LibSndFile on Windows**: No info here yet.
* **LibSndFile on MacOS**: Install using `brew install libsndfile`. The build script(s) takes care of the rest (adjust script if paths differ).
* **LibSndFile on Linux**: Use whatever package management system that is available on your distro.
