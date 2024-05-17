//
//  Synthesizer.h
//  8Beat
//
//  Created by Rasmus Anthin on 2024-02-22.
//

#pragma once

#include "WaveformGeneration.h"
#include "WaveformHelper.h"


namespace audio
{
  enum class InstrumentType
  {
    PIANO,
    VIOLIN,
    ORGAN,
    TRUMPET,
    FLUTE,
    GUITAR,
    KICKDRUM,
    SNAREDRUM,
    HIHAT,
    ANVIL,
    NUM_ITEMS
  };

  class Synthesizer
  {
  public:
  
    static void print_instrument(InstrumentType instr)
    {
      switch (instr)
      {
        case InstrumentType::PIANO: std::cout << "PIANO\n"; break;
        case InstrumentType::VIOLIN: std::cout << "VIOLIN\n"; break;
        case InstrumentType::ORGAN: std::cout << "ORGAN\n"; break;
        case InstrumentType::TRUMPET: std::cout << "TRUMPET\n"; break;
        case InstrumentType::FLUTE: std::cout << "FLUTE\n"; break;
        case InstrumentType::GUITAR: std::cout << "GUITAR\n"; break;
        case InstrumentType::KICKDRUM: std::cout << "KICKDRUM\n"; break;
        case InstrumentType::SNAREDRUM: std::cout << "SNAREDRUM\n"; break;
        case InstrumentType::HIHAT: std::cout << "HIHAT\n"; break;
        case InstrumentType::ANVIL: std::cout << "ANVIL\n"; break;
        default: std::cout << "Unknown Instrument\n"; break;
      }
    }
    
    static Waveform synthesize(const std::vector<std::pair<float, Waveform>>& wave_comp, const ADSR& adsr, const FilterArgs& final_filter_args, bool final_normalize)
    {
      auto wave = WaveformHelper::mix(wave_comp);
      wave = WaveformHelper::envelope_adsr(wave, adsr);
      wave = WaveformHelper::filter(wave, final_filter_args);
      if (final_normalize)
        WaveformHelper::normalize(wave);
      return wave;
    }
  
    static Waveform synthesize(InstrumentType instr,
      const WaveformGeneration& wave_gen,
      float duration_s, float frequency_Hz,
      int sample_rate = 44100,
      bool verbosity = false,
      FrequencyType frequency_effect = FrequencyType::CONSTANT,
      AmplitudeType amplitude_effect = AmplitudeType::CONSTANT,
      PhaseType phase_effect = PhaseType::ZERO)
    {
      ADSR adsr;
      WaveformGenerationParams params;
      Waveform noise, square, triangle, sine, sawtooth;
      std::vector<std::pair<float, Waveform>> wave_comp; // {{ Weight, Waveform }}
      int num_harmonics = 6;
      float tot_harmonics_ampl = 0.f;
      FilterArgs final_filter_args;
      bool final_normalize = false;
      switch (instr)
      {
        case InstrumentType::PIANO:
          adsr = adsr_presets::PIANO_0;
          wave_comp.emplace_back(0.4f, wave_gen.generate_waveform(WaveformType::SINE,
            duration_s, frequency_Hz,
            params, sample_rate, verbosity,
            frequency_effect, amplitude_effect, phase_effect));
          wave_comp.emplace_back(0.3f, wave_gen.generate_waveform(WaveformType::SQUARE,
            duration_s, frequency_Hz));
          wave_comp.emplace_back(0.15f, wave_gen.generate_waveform(WaveformType::TRIANGLE,
            duration_s, frequency_Hz));
          wave_comp.emplace_back(0.15f, wave_gen.generate_waveform(WaveformType::SAWTOOTH,
            duration_s, 3*frequency_Hz));
          break;
        case InstrumentType::VIOLIN:
          adsr = adsr_presets::VIOLIN_0;
          wave_comp.emplace_back(0.5f, wave_gen.generate_waveform(WaveformType::SAWTOOTH,
            duration_s, frequency_Hz, params, sample_rate, verbosity,
            frequency_effect, amplitude_effect, phase_effect));
          wave_comp.emplace_back(0.3f, wave_gen.generate_waveform(WaveformType::SQUARE,
            duration_s, frequency_Hz));
          noise = wave_gen.generate_waveform(WaveformType::NOISE, duration_s);
          noise = WaveformHelper::filter(noise, FilterType::ChebyshevTypeI, FilterOpType::LowPass, 2, 0.9f, 0.1f);
          wave_comp.emplace_back(0.2f, noise);
          final_filter_args.filter_type = FilterType::NONE;
          //final_filter_args.filter_op_type = FilterOpType::LowPass;
          //final_filter_args.filter_order = 2;
          //final_filter_args.cutoff_freq_multiplier = 0.5f;
          //final_filter_args.ripple = 0.2f;
          final_normalize = true;
          break;
        case InstrumentType::ORGAN:
          adsr = adsr_presets::ORGAN_0;
          wave_comp.emplace_back(0.5f, wave_gen.generate_waveform(WaveformType::SINE, duration_s, frequency_Hz, params, sample_rate, verbosity,
              frequency_effect, amplitude_effect, [](float t, float) { return 25.f*t; }));
          wave_comp.emplace_back(0.3f, wave_gen.generate_waveform(WaveformType::SQUARE, duration_s, 2*frequency_Hz,
              params, sample_rate, verbosity,
              FrequencyType::CONSTANT, AmplitudeType::CONSTANT, [](float t, float) { return 30.f*t; }));
          wave_comp.emplace_back(0.1f, wave_gen.generate_waveform(WaveformType::TRIANGLE, duration_s, 3*frequency_Hz));
          wave_comp.emplace_back(0.1f, wave_gen.generate_waveform(WaveformType::SAWTOOTH, duration_s, 4*frequency_Hz));
          break;
        case InstrumentType::TRUMPET:
          adsr = adsr_presets::TRUMPET_0;
          params.duty_cycle = 0.1f;
          wave_comp.emplace_back(0.7f, wave_gen.generate_waveform(WaveformType::SQUARE, duration_s, frequency_Hz,
              params, sample_rate, verbosity,
              frequency_effect, amplitude_effect, phase_effect));
          wave_comp.emplace_back(0.2f, wave_gen.generate_waveform(WaveformType::TRIANGLE, duration_s, frequency_Hz));
          
          sine = wave_gen.generate_waveform(WaveformType::SINE, duration_s, 1.011f*frequency_Hz);
          WaveformHelper::scale(sine, 0.4f);
          wave_comp.back().second = WaveformHelper::ring_modulation(wave_comp.back().second, sine);
          
          wave_comp.emplace_back(0.15f, wave_gen.generate_waveform(WaveformType::TRIANGLE, duration_s, 3.f*frequency_Hz));
          wave_comp.emplace_back(0.05f, wave_gen.generate_waveform(WaveformType::NOISE, duration_s, frequency_Hz));
          break;
        case InstrumentType::FLUTE:
          adsr = adsr_presets::FLUTE_0;
          for (int h = 1; h <= num_harmonics; ++h) tot_harmonics_ampl += 1.f/h;
          for (int h = 1; h <= num_harmonics; ++h)
          {
            auto wdh = wave_gen.generate_waveform(WaveformType::SINE, duration_s, h * frequency_Hz,
                params, sample_rate, verbosity,
                frequency_effect, amplitude_effect, phase_effect);
            wave_comp.emplace_back(0.8f*(1.f/h)/tot_harmonics_ampl, wdh);
          }

#if 0
          triangle = wave_gen.generate_waveform(WaveformType::TRIANGLE, duration_s, 6.f*frequency_Hz);
          triangle = WaveformHelper::filter(triangle, FilterType::ChebyshevTypeII, FilterOpType::LowPass, 1, 2.5f, std::nullopt, 0.1f);
          wave_comp.emplace_back(0.05f, triangle);
#endif
          
          noise = wave_gen.generate_waveform(WaveformType::NOISE,
            duration_s, std::nullopt);
          noise = WaveformHelper::filter(noise, FilterType::ChebyshevTypeI, FilterOpType::LowPass, 2, 0.9f, 0.1f);
          wave_comp.emplace_back(0.1f, noise);
          
          final_filter_args.filter_type = FilterType::Butterworth;
          final_filter_args.filter_op_type = FilterOpType::LowPass;
          final_filter_args.filter_order = 1;
          final_filter_args.cutoff_freq_multiplier = 7.5f;
          final_filter_args.ripple = 0.2f;
          final_normalize = true;
          break;
        case InstrumentType::GUITAR:
          adsr = adsr_presets::GUITAR;
          wave_comp.emplace_back(1.f, WaveformHelper::karplus_strong(duration_s, frequency_Hz));
          wave_comp.emplace_back(0.08f, wave_gen.generate_waveform(WaveformType::NOISE,
            duration_s, frequency_Hz));
          final_filter_args.filter_type = FilterType::NONE;
          //final_filter_args.filter_op_type = FilterOpType::LowPass;
          //final_filter_args.filter_order = 1;
          //final_filter_args.cutoff_freq_multiplier = 1.5f;
          //final_filter_args.ripple = 0.5f;
          final_normalize = true;
          break;
        case InstrumentType::KICKDRUM:
          adsr = adsr_presets::KICKDRUM;
          wave_comp.emplace_back(0.2f, wave_gen.generate_waveform(WaveformType::SINE, duration_s, frequency_Hz));
          wave_comp.emplace_back(0.1f, wave_gen.generate_waveform(WaveformType::TRIANGLE, duration_s, 2*frequency_Hz));
          noise = wave_gen.generate_waveform(WaveformType::NOISE,
            duration_s, frequency_Hz);
          //noise = WaveformHelper::filter(noise, FilterType::ChebyshevTypeI, FilterOpType::LowPass, 2, 0.9f*frequency_Hz, std::nullopt, 0.1f);
          wave_comp.emplace_back(0.4f, noise);
          break;
        case InstrumentType::SNAREDRUM:
          adsr = adsr_presets::SNAREDRUM;
          noise = wave_gen.generate_waveform(WaveformType::NOISE,
            duration_s, std::nullopt);
          wave_comp.emplace_back(0.7f, noise);
          sawtooth = wave_gen.generate_waveform(WaveformType::SAWTOOTH, duration_s, frequency_Hz);
          square = wave_gen.generate_waveform(WaveformType::SQUARE, duration_s, frequency_Hz*1.1f,
            params, sample_rate, verbosity,
            FrequencyType::CONSTANT, AmplitudeType::CONSTANT,
            [](float t, float /*dur*/) { return math::c_2pi*2.13f*t*(1 + 3.f*t); });
          wave_comp.emplace_back(0.25f, WaveformHelper::ring_modulation(sawtooth, square));
          wave_comp.emplace_back(0.05f, wave_gen.generate_waveform(WaveformType::TRIANGLE, duration_s, 3.f*frequency_Hz));
          break;
        case InstrumentType::HIHAT:
          adsr = adsr_presets::HIHAT;
          wave_comp.emplace_back(0.8f, wave_gen.generate_waveform(WaveformType::NOISE, duration_s));
          wave_comp.emplace_back(0.1f, wave_gen.generate_waveform(WaveformType::SINE, duration_s, frequency_Hz));
          wave_comp.emplace_back(0.1f, wave_gen.generate_waveform(WaveformType::TRIANGLE, duration_s, 17.f/3.f*frequency_Hz));
          break;
        case InstrumentType::ANVIL:
          adsr = adsr_presets::SNAREDRUM;
          noise = wave_gen.generate_waveform(WaveformType::NOISE, duration_s);
          wave_comp.emplace_back(0.6f, noise);
          wave_comp.emplace_back(0.15f, wave_gen.generate_waveform(WaveformType::SINE, duration_s, 3.11f*frequency_Hz));
          wave_comp.emplace_back(0.25f, wave_gen.generate_waveform(WaveformType::TRIANGLE, duration_s, 5.43f*frequency_Hz));
          break;
        case InstrumentType::NUM_ITEMS:
          break;
      }
      auto wave = synthesize(wave_comp, adsr, final_filter_args, final_normalize);
      return wave;
    }
    
  };
  
}
