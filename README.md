# 8Beat

Documentation is a work in progress for the moment being. For example on how to use this lib, please clone repo [`Pilot Episode`](https://github.com/razterizer/Pilot-Episode) and follow the instructions there.

## Definitions

We'll often use the term `waveform` as meaning an audio signal here.

## The Library

8Beat is a header-only library that contains the following header files:

* `Waveform.h` contains the struct `Waveform` that contains an audio buffer, and variables `frequency`, `sample_rate` and `duration`. This struct is very central as it holds the PCM audio waveform representation itself.
* `WaveformIO.h`. contains class `WaveformIO` that has two static public functions: `load()` and `save()`. These functions rely on the [`sndfile`](https://github.com/libsndfile/libsndfile) library which allows you to import and export a `Waveform` object to many different types and formats.
* `WaveformGeneration.h` contains class `WaveformGeneration` with a single public function `generate_waveform()`.
* `Spectrum.h` contains struct `Spectrum` which is used in conjunction with functions such as public functions `fft()` and `ifft()` in class `WaveformHelper`.
* `WaveformHelper.h` contains class `WaveformHelper` which has the following public static functions:
  * `mix()` mixes two waveforms by lerping them or multiple waveforms by weighted average.
  * `ring_modulation()` multiplies two waveforms.
  * `reverb()` does reverb between a waveform and an impulse response waveform of an environment (response sound from a dirac pulse-like "trigger" sound) to create a reverb effect.
  * `reverb_fast()` same as `reverb()` but is very fast because it uses the fast Fourier transform.
  * `complex2real()` lets you choose if you want the real part, imag part or absolute value of both from a given complex value.
  * `fft()` this is the fast Fourier transform using the Cooley-Tukey algorithm.
  * `ifft()` this is the fast inverse Fourier transform using a variant of the Cooley-Tukey algorithm.
  * `find_min_max()` finds the min and max values of a given audio signal.
  * `normalize_over()` 
  * `scale()`
  * `clamp()`
  * `fir_moving_average()`
  * `fir_sinc_window_low_pass()`
  * `karplus_strong()`
  * `envelope_adsr()`
  * `resample()`
  * `filter_low_pass()`
  * `print_waveform_graph()`
  * `calc_time_from_num_cycles()`
  * `calc_dt()`
  * `calc_duration()`
  * `calc_num_samples()`
* `ADSR.h`
* `Synthesizer.h`
* `AudioSourceHandler.h`
* `ChipTuneEngine.h`


## Getting Started

This header-only library depends on the header-only library Core-Lib which should be located in a checkout-folder called [`Core Lib`](https://github.com/razterizer/Core-Lib) next to the checkout-folder for this lib (which I recommend you call `8-Bit Audio Emulator Lib`). I intend to change these paths names in the near future. The header only libs uses relative include paths (which is a bit suboptimal), but I'll see if I can find a better solution for this.


