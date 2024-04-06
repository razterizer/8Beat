//
//  WaveformGeneration.h
//  8Beat
//
//  Created by Rasmus Anthin on 2024-02-10.
//

#pragma once

#include "Waveform.h"
#include <vector>

// phi, param
#define WAVEFORM_FUNC_ARGS float, float
// t, duration, frequency_0
#define FREQUENCY_FUNC_ARGS float, float, float
// t, duration
#define AMPLITUDE_FUNC_ARGS float, float
// t, duration
#define PHASE_FUNC_ARGS float, float


namespace audio
{
  enum class WaveformType { SINE, SQUARE, TRIANGLE, SAWTOOTH, NOISE, PWM };
  enum class FrequencyType { CONSTANT, JET_ENGINE_POWERUP, CHIRP_0, CHIRP_1, CHIRP_2 };
  enum class AmplitudeType { CONSTANT, JET_ENGINE_POWERUP, VIBRATO_0 };
  enum class PhaseType { ZERO };
  
  struct ArpeggioPair
  {
    ArpeggioPair(float t, float fm) : time(t), freq_mult(fm) {}
    float time = 0.f;
    float freq_mult = 1.f;
  };
  struct WaveformGenerationParams
  {
    float pwm_duty_cycle = 0.5f;
    std::optional<float> min_frequency_limit = std::nullopt;
    std::optional<float> freq_slide_vel = std::nullopt; // 8va/s
    std::optional<float> freq_slide_acc = std::nullopt; // 8va/s^2
    std::optional<float> vibrato_depth = std::nullopt;
    std::optional<float> vibrato_vel = std::nullopt;
    std::optional<float> vibrato_acc = std::nullopt;
    std::optional<float> vibrato_acc_max_vel_limit = std::nullopt;
    //0.8f + 0.2f*std::sin(math::c_2pi * 2.2f*t*(1 + std::min(0.8f, 0.4f*t)));
    //0.8f + 0.2f*std::sin(math::c_2pi * 2.2f*t + std::min(2.2f*0.8f*t, 2.2f*0.4f*t^2))
    //(1 - vibrato_depth) + vibrato_depth*sin(2pi * vibrato_vel*t + std::min(vibrato_acc_vel_limit*t, 0.5f*vibrato_acc*t*t)
    std::vector<ArpeggioPair> arpeggio;
  };
  
  class WaveformGeneration
  {
  public:
    using WaveformFunc = std::function<float(WAVEFORM_FUNC_ARGS)>;
    using FrequencyFunc = std::function<float(FREQUENCY_FUNC_ARGS)>;
    using AmplitudeFunc = std::function<float(AMPLITUDE_FUNC_ARGS)>;
    using PhaseFunc = std::function<float(PHASE_FUNC_ARGS)>;
    using WaveformFuncArg = std::variant<WaveformType, WaveformFunc>;
    using FrequencyFuncArg = std::variant<FrequencyType, FrequencyFunc>;
    using AmplitudeFuncArg = std::variant<AmplitudeType, AmplitudeFunc>;
    using PhaseFuncArg = std::variant<PhaseType, PhaseFunc>;
    
    // Function to generate a simple waveform buffer
    Waveform generate_waveform(const WaveformFuncArg& wave_func_arg = WaveformType::SINE,
                               float duration = 10.f, float frequency = 440.f,
                               const FrequencyFuncArg& freq_func_arg = FrequencyType::CONSTANT,
                               const AmplitudeFuncArg& ampl_func_arg = AmplitudeType::CONSTANT,
                               const PhaseFuncArg& phase_func_arg = PhaseType::ZERO,
                               WaveformGenerationParams params = {},
                               int sample_rate = 44100,
                               bool verbose = false) const
    {
      Waveform wd;
      wd.frequency = frequency;
      wd.sample_rate = sample_rate;
      wd.duration = duration;
      
      auto buffer_len = static_cast<int>(duration * sample_rate);
      float freq_mod = frequency;
      float ampl_mod = 1.f;
      
      wd.buffer.resize(buffer_len);
      
      // Argument Functions
      auto wave_func = extract_waveform_func(wave_func_arg, verbose);
      auto freq_func = extract_frequency_func(freq_func_arg, verbose);
      auto ampl_func = extract_amplitude_func(ampl_func_arg, verbose);
      auto phase_func = extract_phase_func(phase_func_arg, verbose);
      
      double accumulated_frequency = 0.0;
      
      float param = 0.f;
      param = params.pwm_duty_cycle; // #FIXME: Only set when using PWM waveform.
      
      float t_prev = 0.f;
      float t = 0.f;
      stlutils::sort(params.arpeggio,
        [](const auto& ap1, const auto& ap2) { return ap1.time < ap2.time; });
      for (int i = 0; i < buffer_len; ++i)
      {
        t_prev = t;
        t = static_cast<float>(i) / sample_rate;
        
        freq_mod = freq_func(t, duration, frequency);
        freq_mod *= std::pow(2.0, (params.freq_slide_vel.value_or(0.f) + 0.5f*params.freq_slide_acc.value_or(0.f) * t) * t);
        for (auto& ap : params.arpeggio)
          if (ap.time <= t)
            freq_mod *= ap.freq_mult;
        // Ensure frequency doesn't go below min_frequency_cutoff
        if (params.min_frequency_limit.has_value())
          math::maximize(freq_mod, params.min_frequency_limit.value());
        
        ampl_mod = ampl_func(t, duration);
        if (params.vibrato_depth.has_value())
        {
          float vib_depth = params.vibrato_depth.value_or(0.f);
          float vib_vel = params.vibrato_vel.value_or(0.f);
          float vib_acc = params.vibrato_acc.value_or(0.f);
          float vib_acc_term = 0.5f*vib_acc*t;
          if (params.vibrato_acc_max_vel_limit.has_value())
            math::minimize(vib_acc_term, params.vibrato_acc_max_vel_limit.value());
          float vibrato = (1.f - vib_depth) + vib_depth*std::sin(math::c_2pi * vib_vel*t + vib_acc_term*t);
          ampl_mod *= vibrato;
        }
        
        // Accumulate frequency for phase modulation
        accumulated_frequency += freq_mod;
        
        // Apply phase modulation similar to Octave code
        float phi = phase_func(t, duration);
        auto phase_modulation = static_cast<float>(math::c_2pi * accumulated_frequency / sample_rate + phi);
        float sample = ampl_mod * wave_func(phase_modulation, param);
        wd.buffer[i] = sample;
      }
      
      return wd;
    }
    
  private:
    WaveformFunc extract_waveform_func(const WaveformFuncArg& wave_func_arg, bool verbose) const
    {
      WaveformFunc wave_func = waveform_sine;
      std::visit([&wave_func, this, verbose](auto&& val)
      {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, WaveformType>)
        {
          // Handle enum class case
          if (verbose) std::cout << "WaveformType: ";
          switch (val)
          {
            case WaveformType::SINE:
              wave_func = waveform_sine;
              if (verbose) std::cout << "SINE" << std::endl;
              break;
            case WaveformType::SQUARE:
              wave_func = waveform_square;
              if (verbose) std::cout << "SQUARE" << std::endl;
              break;
            case WaveformType::TRIANGLE:
              wave_func = waveform_triangle;
              if (verbose) std::cout << "TRIANGLE" << std::endl;
              break;
            case WaveformType::SAWTOOTH:
              wave_func = waveform_sawtooth;
              if (verbose) std::cout << "SAWTOOTH" << std::endl;
              break;
            case WaveformType::NOISE:
              wave_func = waveform_noise;
              if (verbose) std::cout << "NOISE" << std::endl;
              break;
            case WaveformType::PWM:
              wave_func = waveform_pwm;
              if (verbose) std::cout << "PWM" << std::endl;
              break;
          }
        }
        else if constexpr (std::is_invocable_v<T, WAVEFORM_FUNC_ARGS>)
        {
          // Handle std::function case
          wave_func = val;
          if (verbose) std::cout << "WaveformType: Custom" << std::endl;
        }
      }, wave_func_arg);
      return wave_func;
    }
    
    FrequencyFunc extract_frequency_func(const FrequencyFuncArg& freq_func_arg, bool verbose) const
    {
      FrequencyFunc freq_func = freq_func_constant;
      std::visit([&freq_func, this, verbose](auto&& val)
      {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, FrequencyType>)
        {
          // Handle enum class case
          if (verbose) std::cout << "FrequencyType: ";
          switch (val)
          {
            case FrequencyType::CONSTANT:
              freq_func = freq_func_constant;
              if (verbose) std::cout << "CONSTANT" << std::endl;
              break;
            case FrequencyType::JET_ENGINE_POWERUP:
              freq_func = freq_func_jet_engine_powerup;
              if (verbose) std::cout << "JET_ENGINE_POWERUP" << std::endl;
              break;
            case FrequencyType::CHIRP_0:
              freq_func = freq_func_chirp_0;
              if (verbose) std::cout << "CHIRP_0" << std::endl;
              break;
            case FrequencyType::CHIRP_1:
              freq_func = freq_func_chirp_1;
              if (verbose) std::cout << "CHIRP_1" << std::endl;
              break;
            case FrequencyType::CHIRP_2:
              freq_func = freq_func_chirp_2;
              if (verbose) std::cout << "CHIRP_2" << std::endl;
              break;
          }
        }
        else if constexpr (std::is_invocable_v<T, FREQUENCY_FUNC_ARGS>)
        {
          // Handle std::function case
          freq_func = val;
          if (verbose) std::cout << "FrequencyType: Custom" << std::endl;
        }
      }, freq_func_arg);
      return freq_func;
    }
    
    AmplitudeFunc extract_amplitude_func(const AmplitudeFuncArg& ampl_func_arg, bool verbose) const
    {
      AmplitudeFunc ampl_func = ampl_func_constant;
      std::visit([&ampl_func, this, verbose](auto&& val)
      {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, AmplitudeType>)
        {
          // Handle enum class case
          if (verbose) std::cout << "AmplitudeType: ";
          switch (val)
          {
            case AmplitudeType::CONSTANT:
              ampl_func = ampl_func_constant;
              if (verbose) std::cout << "CONSTANT" << std::endl;
              break;
            case AmplitudeType::JET_ENGINE_POWERUP:
              ampl_func = ampl_func_jet_engine_powerup;
              if (verbose) std::cout << "JET_ENGINE_POWERUP" << std::endl;
              break;
            case AmplitudeType::VIBRATO_0:
              ampl_func = ampl_func_vibrato_0;
              if (verbose) std::cout << "VIBRATO_0" << std::endl;
              break;
          }
        }
        else if constexpr (std::is_invocable_v<T, AMPLITUDE_FUNC_ARGS>)
        {
          // Handle std::function case
          ampl_func = val;
          if (verbose) std::cout << "AmplitudeType: Custom" << std::endl;
        }
      }, ampl_func_arg);
      return ampl_func;
    }
    
    PhaseFunc extract_phase_func(const PhaseFuncArg& phase_func_arg, bool verbose) const
    {
      PhaseFunc phase_func = phase_func_zero;
      std::visit([&phase_func, this, verbose](auto&& val)
      {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, PhaseType>)
        {
          // Handle enum class case
          if (verbose) std::cout << "PhaseType: ";
          switch (val)
          {
            case PhaseType::ZERO:
              phase_func = phase_func_zero;
              if (verbose) std::cout << "ZERO" << std::endl;
              break;
          }
        }
        else if constexpr (std::is_invocable_v<T, PHASE_FUNC_ARGS>)
        {
          // Handle std::function case
          phase_func = val;
          if (verbose) std::cout << "PhaseType: Custom" << std::endl;
        }
      }, phase_func_arg);
      return phase_func;
    }
    
    // /////////////////////
    // Waveform Functions //
    // /////////////////////
    const WaveformFunc waveform_sine = [](float phi, float /*param*/) -> float
    {
      //return args.amplitude * std::sin(2 * M_PI * args.frequency * t);
      return std::sin(phi);
    };
    
    const WaveformFunc waveform_square = [](float phi, float /*param*/) -> float
    {
#if false
      float f = args.frequency;
      //return args.amplitude * sin(w * t);
      auto a = std::fmod(f * t, 1.f);
#else
      auto a = std::fmod(phi / math::c_2pi, 1.f);
#endif
      if (0 <= a && a < 0.5f)
        return +1.f;
      else
        return -1.f;
    };
    
    const WaveformFunc waveform_triangle = [](float phi, float /*param*/) -> float
    {
#if false
      float f = args.frequency;
      //return args.amplitude * sin(w * t);
      auto a = std::fmod(f * t, 1.f);
#else
      auto a = std::fmod(phi / math::c_2pi, 1.f);
#endif
      if (0 <= a && a < 0.5f)
        return math::lerp(2*a, -1.f, +1.f);
      else
        return math::lerp(2*a-1, +1.f, -1.f);
    };
    
    const WaveformFunc waveform_sawtooth = [](float phi, float /*param*/) -> float
    {
#if false
      float f = args.frequency;
      //return args.amplitude * sin(w * t);
      auto a = std::fmod(f * t, 1.f);
#else
      auto a = std::fmod(phi / math::c_2pi, 1.f);
#endif
      return 2*a-1;
    };
    
    const WaveformFunc waveform_noise = [](float phi, float /*param*/) -> float
    {
      return rnd::rand()*2.0f - 1.0f;
    };
    
    const WaveformFunc waveform_pwm = [](float phi, float param) -> float
    {
      auto duty_cycle = param;
      auto a = std::fmod(phi / math::c_2pi, 1.f);
      if (0 <= a && a < duty_cycle)
        return +1.f;
      else
        return 0.f;
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
    
    const FrequencyFunc freq_func_chirp_0 = [](float t, float duration, float freq_0)
    {
      return freq_0 + 0.5f*t;
    };
    
    const FrequencyFunc freq_func_chirp_1 = [](float t, float duration, float freq_0)
    {
      return freq_0 + 1.5f*t;
    };
    
    const FrequencyFunc freq_func_chirp_2 = [](float t, float duration, float freq_0)
    {
      return freq_0 + 4.f*t;
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
    
    const AmplitudeFunc ampl_func_vibrato_0 = [](float t, float duration)
    {
      return 0.8f + 0.2f*std::sin(math::c_2pi * 2.2f*t*(1 + std::min(0.8f, 0.4f*t)));
    };
    
    // //////////////////
    // Phase Functions //
    // //////////////////
    
    const PhaseFunc phase_func_zero = [](float t, float duration)
    {
      return 0.f;
    };

  };
  
}
