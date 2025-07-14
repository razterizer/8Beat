//
//  WaveformGeneration.h
//  8Beat
//
//  Created by Rasmus Anthin on 2024-02-10.
//

#pragma once

#include "Waveform.h"
#include "WaveformHelper.h"
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
  enum class WaveformType { SINE, SQUARE, TRIANGLE, SAWTOOTH, NOISE };
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
    std::optional<float> vibrato_depth = std::nullopt;
    std::optional<float> vibrato_freq = std::nullopt;
    std::optional<float> vibrato_freq_vel = std::nullopt;
    std::optional<float> vibrato_freq_acc = std::nullopt;
    std::optional<float> vibrato_freq_acc_max_vel_limit = std::nullopt;
    int noise_filter_order = 2;
    float noise_filter_rel_bw = 0.2f;
    float noise_filter_slot_dur_s = 1e-2f;
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
                               float duration = 10.f, std::optional<float> frequency = 440.f,
                               WaveformGenerationParams params = {},
                               int sample_rate = 44100,
                               bool verbose = false,
                               const FrequencyFuncArg& freq_func_arg = FrequencyType::CONSTANT,
                               const AmplitudeFuncArg& ampl_func_arg = AmplitudeType::CONSTANT,
                               const PhaseFuncArg& phase_func_arg = PhaseType::ZERO) const
    {
      Waveform wd;
      auto freq_val = frequency.value_or(440.f);
      wd.frequency = freq_val;
      wd.sample_rate = sample_rate;
      wd.duration = duration;
      
      auto buffer_len = static_cast<int>(duration * sample_rate);
      float ampl_mod = 1.f;
      
      wd.buffer.resize(buffer_len);
      
      // Argument Functions
      auto [wave_func, wave_enum] = extract_waveform_func(wave_func_arg, verbose);
      auto freq_func = extract_frequency_func(freq_func_arg, verbose);
      auto ampl_func = extract_amplitude_func(ampl_func_arg, verbose);
      auto phase_func = extract_phase_func(phase_func_arg, verbose);
      
      double accumulated_frequency = 0.0;
      
      float duty_cycle_0 = 0.f;
      bool is_square = (wave_enum == static_cast<int>(WaveformType::SQUARE));
      bool is_triangle = (wave_enum == static_cast<int>(WaveformType::TRIANGLE));
      bool is_sawtooth = (wave_enum == static_cast<int>(WaveformType::SAWTOOTH));
      bool is_noise = (wave_enum == static_cast<int>(WaveformType::NOISE));
      if (is_square || is_triangle || is_sawtooth)
      {
        if (params.duty_cycle.has_value())
          duty_cycle_0 = params.duty_cycle.value();
        else if (is_sawtooth)
          duty_cycle_0 = 1.f;
        else if (is_square || is_triangle)
          duty_cycle_0 = 0.5f;
      }
      float duty_cycle = duty_cycle_0;
      
      float t = 0.f;
      stlutils::sort(params.arpeggio,
        [](const auto& ap1, const auto& ap2) { return ap1.time < ap2.time; });
      if (!params.arpeggio.empty() && params.arpeggio[0].time > 0.f)
        params.arpeggio.insert(params.arpeggio.begin(), { 0.f, 1.f });
      int Narp = static_cast<int>(params.arpeggio.size());
            
      const int N = static_cast<int>(params.noise_filter_slot_dur_s * sample_rate);
      std::vector<float> noise_buffer(N, 0.f);
      
      auto f_calc_vibrato = [](float& mod, float t,
                               const std::optional<float>& vibrato_depth,
                               const std::optional<float>& vibrato_freq,
                               const std::optional<float>& vibrato_freq_vel,
                               const std::optional<float>& vibrato_freq_acc,
                               const std::optional<float>& vibrato_freq_acc_max_vel_limit)
      {
        if (vibrato_depth.has_value())
        {
          float vib_freq = vibrato_freq.value_or(0.f);
          float vib_depth = vibrato_depth.value_or(0.f);
          float vib_freq_vel = vibrato_freq_vel.value_or(0.f);
          float vib_freq_acc = vibrato_freq_acc.value_or(0.f);
          float vib_freq_acc_term = 0.5f*vib_freq_acc*t;
          if (vibrato_freq_acc_max_vel_limit.has_value())
            math::minimize(vib_freq_acc_term, vibrato_freq_acc_max_vel_limit.value());
          vib_freq = std::max(0.f, vib_freq + (vib_freq_vel + vib_freq_acc_term)*t);
          float vibrato = (1.f - vib_depth) + vib_depth*std::sin(math::c_2pi*vib_freq*t);
          mod *= vibrato;
        }
      };
      
      for (int i = 0; i < buffer_len; ++i)
      {
        t = static_cast<float>(i) / sample_rate;
        
        // Frequency
        float freq_mod = freq_func(t, duration, freq_val);
        f_calc_vibrato(freq_mod, t,
                       params.freq_vibrato_depth,
                       params.freq_vibrato_freq,
                       params.freq_vibrato_freq_vel,
                       params.freq_vibrato_freq_acc,
                       params.freq_vibrato_freq_acc_max_vel_limit);
        freq_mod *= static_cast<float>(std::pow(2.0, (params.freq_slide_vel.value_or(0.f) + 0.5f*params.freq_slide_acc.value_or(0.f) * t) * t));
        if (!params.arpeggio.empty())
        {
          for (int a_idx = 0; a_idx < Narp - 1; ++a_idx)
            if (math::in_range<float>(t, params.arpeggio[a_idx].time, params.arpeggio[a_idx + 1].time, Range::ClosedOpen))
              freq_mod *= params.arpeggio[a_idx].freq_mult;
          if (math::in_range<float>(t, params.arpeggio.back().time, {}, Range::ClosedFree))
            freq_mod *= params.arpeggio.back().freq_mult;
        }
        // Ensure frequency doesn't go below min_frequency_cutoff or above max_frequency_cutoff.
        // min_frequency_limit <= freq_mod <= max_frequency_limit.
        if (params.min_frequency_limit.has_value())
          math::maximize(freq_mod, params.min_frequency_limit.value());
        if (params.max_frequency_limit.has_value())
          math::minimize(freq_mod, params.max_frequency_limit.value());
        
        // Amplitude
        ampl_mod = ampl_func(t, duration);
        f_calc_vibrato(ampl_mod, t,
                       params.vibrato_depth,
                       params.vibrato_freq,
                       params.vibrato_freq_vel,
                       params.vibrato_freq_acc,
                       params.vibrato_freq_acc_max_vel_limit);
        
        // Duty Cycle
        if (params.duty_cycle_sweep.has_value())
          duty_cycle = duty_cycle_0 + t * params.duty_cycle_sweep.value();
        const auto dc_eps = 1e-10f;
        duty_cycle = math::clamp(duty_cycle, dc_eps, 1.f - dc_eps);
        
        // Accumulate frequency for phase modulation
        accumulated_frequency += freq_mod;
        
        // Apply phase modulation similar to Octave code
        float phi = phase_func(t, duration);
        auto phase_modulation = static_cast<float>(math::c_2pi * accumulated_frequency / sample_rate + phi);
        float sample = ampl_mod * wave_func(phase_modulation, duty_cycle);
        if (params.sample_range_min.has_value() || params.sample_range_max.has_value())
          sample = math::linmap(sample, -1.f, +1.f, params.sample_range_min.value_or(-1.f), params.sample_range_max.value_or(+1.f));
        if (is_noise && frequency.has_value())
        {
          int imN = i % N;
          noise_buffer[imN] = sample;
          if (imN == N - 1)
          {
            Filter bp_flt = WaveformHelper::create_Butterworth_filter(params.noise_filter_order, FilterOpType::BandPass, freq_mod, params.noise_filter_rel_bw*freq_mod, sample_rate);
            
            noise_buffer = WaveformHelper::filter(noise_buffer, bp_flt);
            for (int j = 0; j < N && i + j < buffer_len; ++j)
              wd.buffer[i + j] = noise_buffer[j];
          }
        }
        else
          wd.buffer[i] = sample; // Regular sample assign.
      }
      
      if (is_noise && frequency.has_value())
        WaveformHelper::normalize(wd);
      
      return wd;
    }
    
  private:
    std::pair<WaveformFunc, int> extract_waveform_func(const WaveformFuncArg& wave_func_arg, bool verbose) const
    {
      int enum_val = -1;
      WaveformFunc wave_func = waveform_sine;
      std::visit([&wave_func, this, &enum_val, verbose](auto&& val)
      {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, WaveformType>)
        {
          enum_val = static_cast<int>(val);
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
          }
        }
        else if constexpr (std::is_invocable_v<T, WAVEFORM_FUNC_ARGS>)
        {
          // Handle std::function case
          wave_func = val;
          if (verbose) std::cout << "WaveformType: Custom" << std::endl;
        }
      }, wave_func_arg);
      return { wave_func, enum_val };
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
    
    const WaveformFunc waveform_square = [](float phi, float param) -> float
    {
      auto duty_cycle = param;
#if false
      float f = args.frequency;
      //return args.amplitude * sin(w * t);
      auto a = std::fmod(f * t, 1.f);
#else
      auto a = std::fmod(phi / math::c_2pi, 1.f);
#endif
      if (0 <= a && a < duty_cycle)
        return +1.f;
      else
        return -1.f;
    };
    
    const WaveformFunc waveform_triangle = [](float phi, float param) -> float
    {
      auto duty_cycle = param;
#if false
      float f = args.frequency;
      //return args.amplitude * sin(w * t);
      auto a = std::fmod(f * t, 1.f);
#else
      auto a = std::fmod(phi / math::c_2pi, 1.f);
#endif
      //if (0 <= a && a < 0.5f)
      //  return math::lerp(2*a, -1.f, +1.f);
      //else
      //  return math::lerp(2*a-1, +1.f, -1.f);
      if (a < duty_cycle)
        return -1.f + 2.f*a/duty_cycle;
      else // if (a >= duty_cycle)
        return 1.f - 2.f*(a - duty_cycle)/(1 - duty_cycle);
    };
    
    const WaveformFunc waveform_sawtooth = [](float phi, float param) -> float
    {
      auto duty_cycle = param;
      // x = max(0, mod(f*t, 1) - (1 - duty_cycle))/duty_cycle;
#if false
      float f = args.frequency;
      //return args.amplitude * sin(w * t);
      auto a = std::fmod(f * t, 1.f);
#else
      auto a = std::fmod(phi / math::c_2pi, 1.f);
#endif
      a = std::max(0.f, a - (1.f - duty_cycle))/duty_cycle;
      return 2*a-1;
    };
    
    const WaveformFunc waveform_noise = [](float phi, float /*param*/) -> float
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
