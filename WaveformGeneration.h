//
//  WaveformGeneration.h
//  8-Bit Audio Emulator Lib
//
//  Created by Rasmus Anthin on 2024-02-10.
//

#pragma once
#include "Waveform.h"

// phi
#define WAVEFORM_FUNC_ARGS float
// t, duration, frequency_0
#define FREQUENCY_FUNC_ARGS float, float, float
// t, duration
#define AMPLITUDE_FUNC_ARGS float, float


namespace audio
{
  enum class WaveformType { SINE_WAVE, SQUARE_WAVE, TRIANGLE_WAVE, SAWTOOTH_WAVE, NOISE };
  enum class FrequencyType { CONSTANT, JET_ENGINE_POWERUP };
  enum class AmplitudeType { CONSTANT, JET_ENGINE_POWERUP };
  
  class WaveformGeneration
  {
  public:
    using WaveformFunc = std::function<float(WAVEFORM_FUNC_ARGS)>;
    using FrequencyFunc = std::function<float(FREQUENCY_FUNC_ARGS)>;
    using AmplitudeFunc = std::function<float(AMPLITUDE_FUNC_ARGS)>;
    using WaveformFuncArg = std::variant<WaveformType, WaveformFunc>;
    using FrequencyFuncArg = std::variant<FrequencyType, FrequencyFunc>;
    using AmplitudeFuncArg = std::variant<AmplitudeType, AmplitudeFunc>;
    
    // Function to generate a simple waveform buffer
    Waveform generate_waveform(const WaveformFuncArg& wave_func_arg = WaveformType::SINE_WAVE,
                                   float duration = 10.f, float frequency = 440.f,
                                   const FrequencyFuncArg& freq_func_arg = FrequencyType::CONSTANT,
                                   const AmplitudeFuncArg& ampl_func_arg = AmplitudeType::CONSTANT,
                                   float sample_rate = 44100.f,
                                   bool verbose = false)
    {
      Waveform wd;
      wd.frequency = frequency;
      wd.sample_rate = sample_rate;
      wd.duration = duration;
      
      float buffer_len = duration * sample_rate;
      float freq_mod = frequency;
      float ampl_mod = 1.f;
      
      wd.buffer.resize(buffer_len);
      
      // Argument Functions
      auto wave_func = extract_waveform_func(wave_func_arg, verbose);
      auto freq_func = extract_frequency_func(freq_func_arg, verbose);
      auto ampl_func = extract_amplitude_func(ampl_func_arg, verbose);
      
      double accumulated_frequency = 0.0;
      
      for (int i = 0; i < buffer_len; ++i)
      {
        float t = static_cast<float>(i) / sample_rate;
        freq_mod = freq_func(t, duration, frequency);
        ampl_mod = ampl_func(t, duration);
        
        // Accumulate frequency for phase modulation
        accumulated_frequency += freq_mod;
        
        // Apply phase modulation similar to Octave code
        float phase_modulation = 2 * M_PI * accumulated_frequency / sample_rate;
        float sample = ampl_mod * wave_func(phase_modulation);
        wd.buffer[i] = sample;
      }
      
      return wd;
    }
    
  private:
    WaveformFunc extract_waveform_func(const WaveformFuncArg& wave_func_arg, bool verbose)
    {
      WaveformFunc wave_func = waveform_sine;
      std::visit([&wave_func, this, verbose](auto&& val)
                 {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, WaveformType>)
        {
          // Handle enum class case
          switch (val)
          {
            case WaveformType::SINE_WAVE:
              wave_func = waveform_sine;
              if (verbose) std::cout << "Waveform: SINE_WAVE" << std::endl;
              break;
            case WaveformType::SQUARE_WAVE:
              wave_func = waveform_square;
              if (verbose) std::cout << "Waveform: SQUARE_WAVE" << std::endl;
              break;
            case WaveformType::TRIANGLE_WAVE:
              wave_func = waveform_triangle;
              if (verbose) std::cout << "Waveform: TRIANGLE_WAVE" << std::endl;
              break;
            case WaveformType::SAWTOOTH_WAVE:
              wave_func = waveform_sawtooth;
              if (verbose) std::cout << "Waveform: SAWTOOTH_WAVE" << std::endl;
              break;
            case WaveformType::NOISE:
              wave_func = waveform_noise;
              if (verbose) std::cout << "Waveform: NOISE" << std::endl;
              break;
          }
        }
        else if constexpr (std::is_invocable_v<T, WAVEFORM_FUNC_ARGS>)
        {
          // Handle std::function case
          wave_func = val;
          if (verbose) std::cout << "Waveform: Custom" << std::endl;
        }
      }, wave_func_arg);
      return wave_func;
    }
    
    FrequencyFunc extract_frequency_func(const FrequencyFuncArg& freq_func_arg, bool verbose)
    {
      FrequencyFunc freq_func = freq_func_constant;
      std::visit([&freq_func, this, verbose](auto&& val)
                 {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, FrequencyType>)
        {
          // Handle enum class case
          switch (val)
          {
            case FrequencyType::CONSTANT:
              freq_func = freq_func_constant;
              if (verbose) std::cout << "Frequency: CONSTANT" << std::endl;
              break;
            case FrequencyType::JET_ENGINE_POWERUP:
              freq_func = freq_func_jet_engine_powerup;
              if (verbose) std::cout << "Frequency: JET_ENGINE_POWERUP" << std::endl;
              break;
          }
        }
        else if constexpr (std::is_invocable_v<T, FREQUENCY_FUNC_ARGS>)
        {
          // Handle std::function case
          freq_func = val;
          if (verbose) std::cout << "Frequency: Custom" << std::endl;
        }
      }, freq_func_arg);
      return freq_func;
    }
    
    AmplitudeFunc extract_amplitude_func(const AmplitudeFuncArg& ampl_func_arg, bool verbose)
    {
      AmplitudeFunc ampl_func = ampl_func_constant;
      std::visit([&ampl_func, this, verbose](auto&& val)
                 {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, AmplitudeType>)
        {
          // Handle enum class case
          switch (val)
          {
            case AmplitudeType::CONSTANT:
              ampl_func = ampl_func_constant;
              if (verbose) std::cout << "Amplitude: CONSTANT" << std::endl;
              break;
            case AmplitudeType::JET_ENGINE_POWERUP:
              ampl_func = ampl_func_jet_engine_powerup;
              if (verbose) std::cout << "Amplitude: JET_ENGINE_POWERUP" << std::endl;
              break;
          }
        }
        else if constexpr (std::is_invocable_v<T, AMPLITUDE_FUNC_ARGS>)
        {
          // Handle std::function case
          ampl_func = val;
          if (verbose) std::cout << "Amplitude: Custom" << std::endl;
        }
      }, ampl_func_arg);
      return ampl_func;
    }
    
    // /////////////////////
    // Waveform Functions //
    // /////////////////////
    const WaveformFunc waveform_sine = [](float phi) -> float
    {
      //return args.amplitude * std::sin(2 * M_PI * args.frequency * t);
      return std::sin(phi);
    };
    
    const WaveformFunc waveform_square = [](float phi) -> float
    {
#if false
      float f = args.frequency;
      //return args.amplitude * sin(w * t);
      auto a = std::fmod(f * t, 1.f);
#else
      auto a = std::fmod(phi / (2*M_PI), 1.f);
#endif
      if (0 <= a && a < 0.5f)
        return +1.f;
      else
        return -1.f;
    };
    
    const WaveformFunc waveform_triangle = [](float phi) -> float
    {
#if false
      float f = args.frequency;
      //return args.amplitude * sin(w * t);
      auto a = std::fmod(f * t, 1.f);
#else
      auto a = std::fmod(phi / (2*M_PI), 1.f);
#endif
      if (0 <= a && a < 0.5f)
        return math::lerp(2*a, -1.f, +1.f);
      else
        return math::lerp(2*a-1, +1.f, -1.f);
    };
    
    const WaveformFunc waveform_sawtooth = [](float phi) -> float
    {
#if false
      float f = args.frequency;
      //return args.amplitude * sin(w * t);
      auto a = std::fmod(f * t, 1.f);
#else
      auto a = std::fmod(phi / (2*M_PI), 1.f);
#endif
      return 2*a-1;
    };
    
    const WaveformFunc waveform_noise = [](float phi) -> float
    {
      return rnd::rand()*2.0f - 1.0f;
    };
    
    // //////////////////////
    // Frequency Functions //
    // //////////////////////
    const FrequencyFunc freq_func_constant = [](float t, float duration, float freq_0)
    {
      return freq_0;
    };
    
    const FrequencyFunc freq_func_jet_engine_powerup = [](float t, float duration, float freq_0)
    {
      return freq_0*(1 + rnd::rand_float(0, 2)*(0.5f + t));
    };
    
    // //////////////////////
    // Amplitude Functions //
    // //////////////////////
    const AmplitudeFunc ampl_func_constant = [](float t, float duration)
    {
      return 1.f;
    };
    
    const AmplitudeFunc ampl_func_jet_engine_powerup = [](float t, float duration)
    {
      return math::linmap(t, 0.f, duration, 0.f, rnd::rand());
    };

    
  };
  
}
