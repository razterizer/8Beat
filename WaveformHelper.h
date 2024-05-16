//
//  WaveformHelper.h
//  8Beat
//
//  Created by Rasmus Anthin on 2024-02-10.
//

#pragma once

#include "Waveform.h"
#include "Spectrum.h"
#include "ADSR.h"

#include "../Core/MachineLearning/ann_cnn.h"
#include "../Core/Math.h"

#include <complex>


namespace audio
{
  using namespace std::complex_literals;

  enum class FilterType { NONE, Butterworth, ChebyshevTypeI };
  enum class FilterOpType { NONE, LowPass, HighPass, BandPass, BandStop };
  enum class GraphType { PLOT_THIN, PLOT_THICK0, PLOT_THICK1, PLOT_THICK2, PLOT_THICK3, FILLED_BOTTOM_UP, FILLED_FROM_T_AXIS };
  enum class Complex2Real { ABS, REAL, IMAG };
  enum class WindowType { HAMMING, HANNING };
  
  struct FilterArgs
  {
    FilterType filter_type = FilterType::NONE;
    FilterOpType filter_op_type = FilterOpType::NONE;
    int filter_order = 1;
    float cutoff_freq_multiplier = 2.5f;
    std::optional<float> bandwidth_freq_multiplier = std::nullopt;
    float ripple = 0.1f;
    bool normalize_filtered_wave = false;
  };
  
  struct Filter
  {
    std::vector<float> a;
    std::vector<float> b;
  };
  
  struct FilterS
  {
    std::vector<std::complex<double>> zeroes, poles;
    double gain = 1.f;
  };
  
  class WaveformHelper
  {
  public:
    static Waveform subset(const Waveform& wave, size_t start_idx, size_t length = static_cast<size_t>(-1))
    {
      auto N = wave.buffer.size();
      if (N == 0)
        return {};
        
      Waveform output;
      output.copy_properties(wave);
      
      if (start_idx < N)
      {
        if (length == static_cast<size_t>(-1))
          length = N;
        
        size_t N_new = length;
        if (start_idx + length > N)
          N_new = N - start_idx;
        output.buffer.resize(N_new);
        std::memcpy(output.buffer.data(), wave.buffer.data() + start_idx, N_new * sizeof(wave.buffer[0]));
      }
        
      output.update_duration();
      return output;
    }
  
    static Waveform mix(const std::vector<std::pair<float, Waveform>>& weighted_waves)
    {
      const size_t Nw = weighted_waves.size();
      size_t Nmin = static_cast<size_t>(-1);
      std::vector<std::pair<float, Waveform>> res_weighted_waves(weighted_waves.size());
      int common_sample_rate = 0;
      float frequency = 0.f;
      float weight_sum = 0.f;
      for (const auto& ww : weighted_waves)
      {
        const auto weight = ww.first;
        const auto& wave = ww.second;
        math::maximize(common_sample_rate, wave.sample_rate);
        frequency += wave.frequency;
        math::minimize(Nmin, wave.buffer.size());
        weight_sum += weight;
      }
      frequency /= Nw;
      for (size_t w_idx = 0; w_idx < Nw; ++w_idx)
      {
        const auto& ww_i = weighted_waves[w_idx];
        const auto& wave_i = ww_i.second;
        auto& ww_o = res_weighted_waves[w_idx];
        ww_o.first = ww_i.first;
        ww_o.second = resample(wave_i, common_sample_rate, FilterType::Butterworth);
      }
    
      Waveform weighted_sum(Nmin, 0.f);
      weighted_sum.sample_rate = common_sample_rate;
      weighted_sum.frequency = frequency;
      
      for (size_t i = 0; i < Nmin; ++i)
      {
        weighted_sum.buffer[i] = 0.f;
        for (const auto& ww : weighted_waves)
          weighted_sum.buffer[i] += ww.first * ww.second.buffer[i];
        weighted_sum.buffer[i] /= weight_sum;
      }
      
      weighted_sum.update_duration();
      return weighted_sum;
    }
  
    static Waveform mix(float t, const Waveform& wave_A, const Waveform& wave_B)
    {
      // Resample both signals to a common sample rate
      int common_sample_rate = std::max(wave_A.sample_rate, wave_B.sample_rate);
      Waveform res_A = resample(wave_A, common_sample_rate, FilterType::Butterworth);
      Waveform res_B = resample(wave_B, common_sample_rate, FilterType::Butterworth);
      
      const auto Nmin = std::min(res_A.buffer.size(), res_A.buffer.size());
      Waveform sum(Nmin, 0.f);
      sum.sample_rate = common_sample_rate;
      sum.frequency = (res_A.frequency + res_B.frequency)*0.5f;
      
      for (size_t i = 0; i < Nmin; ++i)
        sum.buffer[i] = math::lerp(t, res_A.buffer[i], res_B.buffer[i]);
      
      return sum;
    }
  
    static Waveform ring_modulation(const Waveform& wave_A, const Waveform& wave_B)
    {
      // Resample both signals to a common sample rate
      int common_sample_rate = std::max(wave_A.sample_rate, wave_B.sample_rate);
      Waveform res_A = resample(wave_A, common_sample_rate, FilterType::Butterworth);
      Waveform res_B = resample(wave_B, common_sample_rate, FilterType::Butterworth);
      
      
      Waveform prod;
      prod.sample_rate = common_sample_rate;
      prod.frequency = calc_fundamental_frequency(res_A.frequency, res_B.frequency);
      
      int Nmin = static_cast<int>(std::min(res_A.buffer.size(), res_B.buffer.size()));
      prod.buffer.resize(Nmin);
      
      for (int i = 0; i < Nmin; ++i)
      {
        float a = res_A.buffer[i];
        float b = res_B.buffer[i];
        prod.buffer[i] = a * b;
      }
      
      prod.duration = calc_duration(prod);
      
      return prod;
    }
    
    // wave: the waveform to produce a reverb effect for.
    // kernel: a dirac-like pulse sample in the environment to make a reverb effect from.
    static Waveform reverb(const Waveform& wave, const Waveform& kernel)
    {
      // Resample both signals to a common sample rate.
      int common_sample_rate = std::max(wave.sample_rate, kernel.sample_rate);
      Waveform res_wave = resample(wave, common_sample_rate, FilterType::Butterworth);
      Waveform res_kernel = resample(kernel, common_sample_rate, FilterType::Butterworth);
      
      Waveform conv;
      conv.sample_rate = common_sample_rate;
      conv.frequency = calc_fundamental_frequency(res_wave.frequency, res_kernel.frequency);
      
      conv.buffer = ml::ann::conv_1d(res_wave.buffer, res_kernel.buffer, 0.f,
                                       ml::ann::ConvRange::Full, ml::ann::ConvType::Convolution);
                                       
      normalize_over(conv);
      //auto [minval, maxval] = WaveformHelper::find_min_max(conv, true);
      //std::cout << "minval = " << minval << std::endl;
      //std::cout << "maxval = " << maxval << std::endl;
      
      conv.update_duration();
      
      return conv;
    }
    
    static Waveform reverb_fast(const Waveform& wave, const Waveform& kernel)
    {
      // Resample both signals to a common sample rate.
      int common_sample_rate = std::max(wave.sample_rate, kernel.sample_rate);
      Waveform res_wave = resample(wave, common_sample_rate, FilterType::Butterworth);
      Waveform res_kernel = resample(kernel, common_sample_rate, FilterType::Butterworth);
      
      // Apply windowing.
      //apply_window(res_wave, WindowType::HAMMING);
      //apply_window(res_kernel, WindowType::HAMMING);
      
      // Padding to common length.
      auto Nw = static_cast<int>(res_wave.buffer.size());
      auto Nk = static_cast<int>(res_kernel.buffer.size());
      auto N = static_cast<int>(std::max(Nw, Nk));
      for (int i = 0; i < N - Nw; ++i)
        res_wave.buffer.emplace_back(0.f);
      for (int i = 0; i < N - Nk; ++i)
        res_kernel.buffer.emplace_back(0.f);
      
      auto spec_res_wave = fft(res_wave);
      auto spec_res_kernel = fft(res_kernel);
      
      int Ns = static_cast<int>(spec_res_wave.buffer.size());
      Spectrum prod;
      prod.buffer.resize(Ns);
      for (int i = 0; i < Ns; ++i)
        prod.buffer[i] = spec_res_wave.buffer[i] * spec_res_kernel.buffer[i];
      prod.copy_properties(spec_res_wave);
      
      auto reverb = ifft(prod);
      auto conv_full_size = Nw + Nk - 1;
      //reverb.buffer.resize(std::min(reverb.buffer.size(), res_kernel.buffer.size()));
      //reverb.buffer.resize(std::min(reverb.buffer.size(), res_wave.buffer.size()));
      reverb.buffer.resize(conv_full_size);
      reverb.sample_rate = common_sample_rate;
      reverb.frequency = calc_fundamental_frequency(res_wave.frequency, res_kernel.frequency);
      reverb.update_duration();
      
      // Normalize to amplitude max 1 if amplitude > 1.
      normalize_over(reverb);
      //auto [minval, maxval] = WaveformHelper::find_min_max(reverb, true);
      //std::cout << "minval = " << minval << std::endl;
      //std::cout << "maxval = " << maxval << std::endl;
      
      //apply_window(reverb, WindowType::HANNING);
      
      return reverb;
    }
    
    static float complex2real(const std::complex<float>& input, Complex2Real filter)
    {
      switch (filter)
      {
        case Complex2Real::ABS: return std::abs(input);
        case Complex2Real::REAL: return std::real(input);
        case Complex2Real::IMAG: return std::imag(input);
      }
    }
    
    static std::vector<float> complex2real(const std::vector<std::complex<float>>& input,
      Complex2Real filter)
    {
      size_t N = input.size();
      std::vector<float> output(N);
      switch (filter)
      {
        case Complex2Real::ABS:
          for (size_t i = 0; i < N; ++i)
            output[i] = std::abs(input[i]);
          break;
        case Complex2Real::REAL:
          for (size_t i = 0; i < N; ++i)
            output[i] = std::real(input[i]);
          break;
        case Complex2Real::IMAG:
          for (size_t i = 0; i < N; ++i)
            output[i] = std::imag(input[i]);
          break;
      }
      return output;
    }
    
    static Spectrum fft(const Waveform& wave)
    {
      auto sz = static_cast<int>(wave.buffer.size());
      auto N = static_cast<int>(std::pow<int>(2, std::ceil(std::log(sz)/std::log(2))));
    
      std::vector<std::complex<float>> input(std::begin(wave.buffer), std::end(wave.buffer));
      // Padding.
      for (int i = 0; i < N - sz; ++i)
        input.emplace_back(0.f);
      
      auto output = fft_rec(input);
      
      Spectrum result;
      result.buffer = output;
      // Calculate frequency axis.
      result.freq_start = -wave.sample_rate / 2.f;
      result.freq_end = wave.sample_rate / 2.f;
      
      return result;
    }
    
    static Waveform ifft(const Spectrum& spectrum, Complex2Real c2r_filter = Complex2Real::REAL)
    {
      auto sz = static_cast<int>(spectrum.buffer.size());
      auto N = static_cast<int>(std::pow<int>(2, std::ceil(std::log(sz)/std::log(2))));
    
      std::vector<std::complex<float>> input(std::begin(spectrum.buffer), std::end(spectrum.buffer));
      // Padding.
      for (int i = 0; i < N - sz; ++i)
        input.emplace_back(0.f);
      
      auto output = ifft_rec(input);
      
      // Normalize the output.
      for (auto& s : output)
        s /= static_cast<float>(N);
      
      Waveform result;
      result.buffer = complex2real(output, c2r_filter);
      // Calculate frequency axis.
      result.sample_rate = static_cast<int>(spectrum.freq_end * 2.f);
      
      return result;
    }
    
    static std::tuple<float, float> find_min_max(const Waveform& wd, bool abs = false)
    {
      float min_val = 0.f;
      float max_val = 0.f;
      if (abs)
      {
        min_val = +std::numeric_limits<float>::infinity();
        max_val = 0.f;
        for (const auto& s : wd.buffer)
        {
          math::minimize(min_val, std::abs(s));
          math::maximize(max_val, std::abs(s));
        }
      }
      else
      {
        min_val = +std::numeric_limits<float>::infinity();
        max_val = -std::numeric_limits<float>::infinity();
        for (const auto& s : wd.buffer)
        {
          math::minimize(min_val, s);
          math::maximize(max_val, s);
        }
      }

      return { min_val, max_val };
    }
    
    // Normalize only over a certain amplitude limit.
    // if max(abs(waveform)) > amplitude_limit
    //   waveform scaled so that max amplitude = amplitude_limit
    // else
    //   unchanged
    static void normalize_over(Waveform& wd, float amplitude_limit = 1.f)
    {
      auto [_, max_val] = find_min_max(wd, true);
      if (max_val > amplitude_limit)
      {
        for (auto& s : wd.buffer)
          s *= amplitude_limit / max_val;
      }
    }
    
    // Normalize so that max amplitude = 1
    static void normalize(Waveform& wd)
    {
      auto [_, max_val] = find_min_max(wd, true);
      for (auto& s : wd.buffer)
        s /= max_val;
    }
    
    // Scale so that max_amplitude = scale.
    static void normalize_scale(Waveform& wd, float scale = 1.f)
    {
      auto [_, max_val] = find_min_max(wd, true);
      for (auto& s : wd.buffer)
        s *= scale / max_val;
    }
    
    static void scale(Waveform& wd, float scale = 1.f)
    {
      for (auto& s : wd.buffer)
        s *= scale;
    }
    
    static void clamp(Waveform& wd, float min = -1.f, float max = +1.f)
    {
      for (auto& s : wd.buffer)
        if (s > max)
          s = max;
        else if (s < min)
          s = min;
    }
    
    static Waveform fir_moving_average(const Waveform& wave, int window_size, bool preserve_amplitude)
    {
      auto N = static_cast<int>(wave.buffer.size());
      if (window_size > N)
        window_size = N;
      int Navg = N - window_size + 1;
      Waveform output(Navg, 0.f);
      output.copy_properties(wave);
      
      auto [_, max_val] = find_min_max(wave, true);
      
      for (int i = 0; i < Navg; ++i)
      {
        for (int j = 0; j < window_size; ++j)
          output.buffer[i] += wave.buffer[i + j];
        output.buffer[i] /= static_cast<float>(window_size);
      }
      
      if (preserve_amplitude)
        scale(output, max_val);
      
      output.update_duration();
      return output;
    }
    
    static Waveform fir_sinc_window_low_pass(const Waveform& wave)
    {
      auto N = static_cast<int>(wave.buffer.size());
      Waveform output(N, 0.f);
      // #FIXME: Implement me.
      output.update_duration();
      return output;
    }
    
    static Waveform flanger(const Waveform& wave, float delay_time, float rate, float feedback)
    {
      Waveform output = wave;
      
      output.buffer = flanger(wave.buffer, delay_time, rate, feedback, wave.sample_rate);
   
      return output;
    }
    
    // #FIXME: Why you not work!?!?
    static Waveform fir_chorus(const Waveform& wave, float modulation_freq = 1.f, float modulation_depth = 5e-3f, const std::vector<float> coeffs = { 1.f, .5f, -.2f, .1f })
    {
      auto N = wave.buffer.size();
      auto Nc = coeffs.size();
      Waveform output(N, 0.f);
      output.copy_properties(wave);
      
      float delay = modulation_depth * std::sin(math::c_2pi * modulation_freq / wave.sample_rate);
      
      // FIR filter.
      for (size_t i = 0; i < N; ++i)
        for (size_t j = 0; j < Nc; ++j)
        {
          auto delayed_index = static_cast<int>(i - delay * wave.sample_rate * j);
          if (delayed_index >= 0 && delayed_index < static_cast<int>(wave.buffer.size()))
            output.buffer[i] += coeffs[j] * wave.buffer[delayed_index];
        }
        
      output.update_duration();
      return output;
    }
    
    // Emulates string instrument sounds.
    static Waveform karplus_strong(float duration_s, float frequency,
                                   int sample_rate = 44100)
    {
      auto Ns = static_cast<int>(calc_num_samples(duration_s, sample_rate));
      Waveform wave(Ns, 0.f);
      wave.duration = duration_s;
      wave.frequency = frequency;
      
      auto Nb = std::min(Ns, static_cast<int>(std::round(sample_rate / frequency)));
      
      std::vector<float> noise(Nb);
      for (int s_idx = 0; s_idx < Nb; ++s_idx)
        noise[s_idx] = rnd::rand_float(-1.f, +1.f);
      
      // y(n) = x(n) + (y(n-N) + y(n-N+1))/2
      
      for (int s_idx = 0; s_idx < Ns; ++s_idx)
      {
        size_t idx_offs = s_idx < Nb ? s_idx : s_idx - Nb;
        float avg = 0.5f * (wave.buffer[idx_offs % Ns] + wave.buffer[idx_offs % Ns]);
        wave.buffer[s_idx] = avg + noise[s_idx % Nb];
        // Low-pass filter
        //wave.buffer[s_idx] = 0.5f * (wave.buffer[s_idx] + wave.buffer[s_idx - 1]);
      }
      
      return wave;
    }
    
    static Waveform envelope_adsr(const Waveform& wave, const ADSR& adsr)
    {
      auto N = wave.buffer.size();
      Waveform output = wave;
      
      //const auto& attack = adsr.attack;
      //const auto& decay = adsr.decay;
      //const auto& sustain = adsr.sustain;
      //const auto& release = adsr.release;
      
      const float gate_s = output.duration;
      const float attack_s = adsr.get_time_A_ms() * 1e-3f;
      const float decay_s = adsr.get_time_D_ms() * 1e-3f;
      //const float sustain_lvl = sustain.sustain_level;
      const float release_s = adsr.get_time_R_ms() * 1e-3f;
      
      const float t_a = 0.f;
      const float t_adr = std::max(gate_s - release_s, 0.f);
      const float t_ad = std::min(t_adr, attack_s);
      const float t_ds = std::min(t_adr, attack_s + decay_s);
      const float t_sr = std::max(t_ds, t_adr); //gate_ms - attack_ms - decay_ms - release_ms;
      const float t_r = gate_s;
      
      float dt = calc_dt(wave);
      float t = 0.f;
      for (size_t i = 0; i < N; ++i)
      {
        t = i * dt;
        //std::cout << t << '\n';
        auto& s = output.buffer[i];
        float env = 0.f;
        float p = 0.f; // Param.
        float pl = 0.f; // Linear Param.
        
        if (math::in_range<float>(t, t_a, t_ad, Range::Closed))
        {
          //std::cout << "A";
          pl = math::value_to_param(t, t_a, t_ad);
          switch (adsr.get_shape_A())
          {
            case ADSRMode::LIN:
              p = pl;
              //env = math::linmap(t, t_a, t_ad, 0.f, 1.f);
              break;
            case ADSRMode::EXP:
              p = std::exp(pl * math::c_ln2) - 1;
              //env = std::exp(math::c_ln2 * (t - t_a)/attack_s) - 1;
              break;
            case ADSRMode::LOG:
              p = std::log(pl * (math::c_e - 1) + 1);
              //env = std::log((t - t_a)/attack_s*(math::c_e - 1) + 1);
              break;
          }
          env = math::lerp(p, adsr.get_level_A0(), adsr.get_level_A1());
        }
        else if (math::in_range<float>(t, t_ad, t_ds, Range::OpenClosed))
        {
          //std::cout << "D";
          pl = math::value_to_param(t, t_ad, t_ds);
          switch (adsr.get_shape_D())
          {
            case ADSRMode::LIN:
              p = pl;
              //env = math::linmap(t, t_ad, t_ds, 1.f, sustain_lvl);
              break;
            case ADSRMode::EXP:
              p = 2 - 2*std::exp(-pl * math::c_ln2);
              //env = (2 - 2.f*sustain_lvl)*std::exp(-math::c_ln2*(t - t_ad)/decay_s) - (1 - 2.f*sustain_lvl);
              break;
            case ADSRMode::LOG:
              p = -std::log(1 - pl*(1 - math::c_1_e));
              //env = std::log(1-(t - t_ad)/decay_s*(1.f-std::exp(sustain_lvl - 1))) + 1;
              break;
          }
          env = math::lerp(p, adsr.get_level_D0(), adsr.get_level_D1());
        }
        else if (math::in_range<float>(t, t_ds, t_sr, Range::OpenClosed))
        {
          //std::cout << "S";
          env = adsr.get_level_S();
        }
        else if (math::in_range<float>(t, t_sr, t_r, Range::OpenClosed))
        {
          //std::cout << "R";
          pl = math::value_to_param(t, t_sr, t_r);
          switch (adsr.get_shape_R())
          {
            case ADSRMode::LIN:
              p = pl;
              //env = math::linmap(t, t_sr, t_r, sustain_lvl, 0.f);
              break;
            case ADSRMode::EXP:
              p = 2 - 2*std::exp(-pl * math::c_ln2);
              //env = 2.f*sustain_lvl*std::exp(-static_cast<float>(M_LN2)*(t - t_sr)/release_s) - sustain_lvl;
              break;
            case ADSRMode::LOG:
              p = -std::log(1 - pl*(1 - math::c_1_e));
              //env = sustain_lvl*(static_cast<float>(std::log(1-(t - t_sr))/release_s*(1-1./static_cast<float>(M_E))) + 1);
              break;
          }
          env = math::lerp(p, adsr.get_level_R0(), adsr.get_level_R1());
        }
        
        //std::cout << env << ", ";
        s *= env;
      }
      
      output.update_duration();
      return output;
    }
    
    static Waveform envelope_adsr(const Waveform& wave,
      const Attack& attack, const Decay& decay, const Sustain& sustain, const Release& release)
    {
      ADSR adsr(attack, decay, sustain, release);
      return envelope_adsr(wave, adsr);
    }
    
    static Waveform resample(const Waveform& wave, int new_sample_rate = 44100,
                             FilterType filter_type = FilterType::Butterworth,
                             int filter_order = 1, float cutoff_freq_multiplier = 2.5f, float ripple = 0.1f)
    {
      if (wave.sample_rate == new_sample_rate)
        return wave;
      
      Waveform resampled_wave;
      resampled_wave.copy_properties(wave);
      resampled_wave.sample_rate = new_sample_rate;
      
      // Calculate the resampling factor
      float resampling_factor = static_cast<float>(wave.sample_rate) / new_sample_rate;
      
      //resampled_wave.duration = wave.duration * resampling_factor;
      
      // Calculate the new size of the buffer
      size_t new_size = static_cast<size_t>(wave.buffer.size() / resampling_factor);
      
      // Resize the buffer in the resampled_wave struct
      resampled_wave.buffer.resize(new_size);
      
      // Perform linear interpolation to fill the new buffer
      for (size_t i = 0; i < new_size; ++i)
      {
        float index_f = i * resampling_factor;
        size_t index = static_cast<size_t>(index_f);
        float fraction = index_f - index;
        
        // Linear interpolation
        resampled_wave.buffer[i] = (1.0f - fraction) * wave.buffer[index]
        + fraction * wave.buffer[index + 1];
      }
      
      auto filtered_wave = filter(resampled_wave, filter_type, FilterOpType::LowPass,
        filter_order, cutoff_freq_multiplier * wave.frequency, ripple, true);
      
      filtered_wave.update_duration();
      return filtered_wave;
    }
    
    static Waveform filter(const Waveform& wave, const FilterArgs& args)
    {
      std::optional<float> bandwidth;
      if (args.bandwidth_freq_multiplier.has_value())
        bandwidth = args.bandwidth_freq_multiplier.value() * wave.frequency;
      return filter(wave,
        args.filter_type,
        args.filter_op_type,
        args.filter_order,
        args.cutoff_freq_multiplier * wave.frequency,
        bandwidth,
        args.ripple,
        args.normalize_filtered_wave);
    }
    
    static Waveform filter(const Waveform& wave,
                           FilterType type,
                           FilterOpType op_type,
                           int filter_order,
                           float freq_cutoff_hz, std::optional<float> freq_bandwidth_hz,
                           float ripple = 0.1f, // ripple: For Chebychev filters.
                           bool normalize_filtered_wave = false)
    {
      auto filtered_wave = wave;
      
      Filter flt;
      
      switch (type)
      {
        case FilterType::Butterworth:
          flt = create_Butterworth_filter(filter_order, op_type, freq_cutoff_hz, freq_bandwidth_hz, wave.sample_rate);
          break;
          
        case FilterType::ChebyshevTypeI:
          flt = create_ChebyshevI_filter(filter_order, op_type, freq_cutoff_hz, freq_bandwidth_hz, ripple, wave.sample_rate);
          break;
          
        default:
          std::cerr << "ERROR in filter(const Waveform&, ...) : Unknown filter type!";
          break;
      }
      
      filtered_wave.buffer = filter(wave.buffer, flt);
      
      clamp(filtered_wave);
      if (normalize_filtered_wave)
        normalize(filtered_wave);
      
      return filtered_wave;
    }
    
    static Waveform filter(const Waveform& wave,
                           const Filter& filter)
    {
      Waveform ret = wave;
      ret.buffer = WaveformHelper::filter(wave.buffer, filter);
      return ret;
    }
    
    static std::vector<float> filter(const std::vector<float>& x,
                                     const Filter& filter)
    {
      size_t Ns = x.size();
      size_t Na = filter.a.size();
      size_t Nb = filter.b.size();
      
      std::vector<float> y(Ns, 0);
      
      // Apply the filter (Direct Form I)
      for (size_t n = 0; n < Ns; ++n)
      {
        y[n] = 0;
        
        // Summing the b terms
        for (size_t i = 0; i < Nb; i++)
        {
          if (n >= i)
            y[n] += filter.b[i] * x[n - i];
        }
        
        // Subtracting the a terms
        for (size_t i = 1; i < Na; i++)
        {
          if (n >= i)
            y[n] -= filter.a[i] * y[n - i];
        }
        
        y[n] /= filter.a[0];  // Normalize output
      }
      
      return y;
    }
    
    static Filter create_Butterworth_filter(int order,
                                            FilterOpType type,
                                            float freq_cutoff, std::optional<float> freq_bandwidth,
                                            int sample_rate = 44100)
    {
      Filter flt;
      
      if (type == FilterOpType::NONE)
        return flt;
      
      if (order <= 0)
      {
        std::cerr << "The order of the Butterworth filter must be at least 1!" << std::endl;
        return flt;
      }
      
      if ((type == FilterOpType::BandPass || type == FilterOpType::BandStop) && !freq_bandwidth.has_value())
      {
        std::cerr << "freq_bandwidth must be specified when creating a BandPass or BandStop filter!" << std::endl;
        return flt;
      }
      
      std::vector<double> W_cutoff;
      if (freq_bandwidth.has_value())
      {
        // 2*(Fc - BW/2)/Fs
        // 2*(Fc + BW/2)/Fs
        W_cutoff.reserve(2);
        W_cutoff.emplace_back(std::max(0.f, (2 * freq_cutoff - freq_bandwidth.value()) / sample_rate));
        W_cutoff.emplace_back((2 * freq_cutoff + freq_bandwidth.value()) / sample_rate);
      }
      else
      {
        W_cutoff.reserve(1);
        W_cutoff.emplace_back(2 * freq_cutoff / sample_rate);
      }
      
      double fs2 = 2.;
      for (auto& wc : W_cutoff)
        wc = 2. / fs2 * std::tan(math::c_pi * wc / fs2);
      
      FilterS s;
      s.poles.reserve(order);
      float v = static_cast<double>(order + 1);
      for (int i = 1; i <= order; ++i)
      {
        s.poles.emplace_back(std::exp(1i * M_PI * (0.5 * v / order)));
        v += 2.;
      }
      // It is supposed to be -1 here. Makes sure the value is clean.
      if (order % 2 == 1)
        s.poles[(order - 1)/2] = -1.;
      
      s.gain = 1.;
      
      s = filter_edge_adjustment(s, type, W_cutoff[0], W_cutoff.size() == 2 ? W_cutoff[1] : 0.f);
      
      bilinear(s, fs2);
      
      flt.b = stlutils::static_cast_vector<float>(stlutils::mult_scalar(poly(s.zeroes), s.gain));
      flt.a = stlutils::static_cast_vector<float>(poly(s.poles));
      
      return flt;
    }
    
    static Filter create_ChebyshevI_filter(int order,
                                           FilterOpType type,
                                           float freq_cutoff, std::optional<float> freq_bandwidth,
                                           float ripple,
                                           int sample_rate = 44100)
    {
      Filter flt;
      
      if (type == FilterOpType::NONE)
        return flt;
      
      if (order <= 0)
      {
        std::cerr << "The order of the Chebyshev type I filter must be at least 1!" << std::endl;
        return flt;
      }
      
      if ((type == FilterOpType::BandPass || type == FilterOpType::BandStop) && !freq_bandwidth.has_value())
      {
        std::cerr << "freq_bandwidth must be specified when creating a BandPass or BandStop filter!" << std::endl;
        return flt;
      }
      
      std::vector<double> W_cutoff;
      if (freq_bandwidth.has_value())
      {
        // 2*(Fc - BW/2)/Fs
        // 2*(Fc + BW/2)/Fs
        W_cutoff.reserve(2);
        W_cutoff.emplace_back(std::max(0.f, (2 * freq_cutoff - freq_bandwidth.value()) / sample_rate));
        W_cutoff.emplace_back((2 * freq_cutoff + freq_bandwidth.value()) / sample_rate);
      }
      else
      {
        W_cutoff.reserve(1);
        W_cutoff.emplace_back(2 * freq_cutoff / sample_rate);
      }
      
      double fs2 = 2.;
      for (auto& wc : W_cutoff)
        wc = 2. / fs2 * std::tan(math::c_pi * wc / fs2);
  
      auto epsilon = std::sqrt(std::pow(10., ripple / 10.) - 1.);
      auto v0 = std::asinh(1. / epsilon) / order;
      
      FilterS s;
      s.poles.reserve(order);
      float v = static_cast<double>(1 - order);
      for (int i = 1; i <= order; ++i)
      {
        auto p = std::exp(1i * M_PI * (0.5 * v / order));
        p = -std::sinh(v0) * std::real(p) + 1i * std::cosh(v0) * std::imag(p);
        s.poles.emplace_back(p);
        v += 2.;
      }
      
      s.gain = std::real(stlutils::prod(stlutils::unary_minus(s.poles)));
      if (order % 2 == 0)
        s.gain *= std::pow(10., -ripple / 20.);
      
      s = filter_edge_adjustment(s, type, W_cutoff[0], W_cutoff.size() == 2 ? W_cutoff[1] : 0.f);
      
      bilinear(s, fs2);
      
      flt.b = stlutils::static_cast_vector<float>(stlutils::mult_scalar(poly(s.zeroes), s.gain));
      flt.a = stlutils::static_cast_vector<float>(poly(s.poles));
      
      return flt;      
    }
    
    static Filter create_ChebyshevII_filter(int order,
                                            FilterOpType type,
                                            float freq_cutoff, std::optional<float> freq_bandwidth,
                                            float ripple,
                                            int sample_rate = 44100)
    {
      Filter flt;
      
      if (type == FilterOpType::NONE)
        return flt;
      
      if (order <= 0)
      {
        std::cerr << "The order of the Chebyshev type I filter must be at least 1!" << std::endl;
        return flt;
      }
      
      if ((type == FilterOpType::BandPass || type == FilterOpType::BandStop) && !freq_bandwidth.has_value())
      {
        std::cerr << "freq_bandwidth must be specified when creating a BandPass or BandStop filter!" << std::endl;
        return flt;
      }
      
      std::vector<double> W_cutoff;
      if (freq_bandwidth.has_value())
      {
        // 2*(Fc - BW/2)/Fs
        // 2*(Fc + BW/2)/Fs
        W_cutoff.reserve(2);
        W_cutoff.emplace_back(std::max(0.f, (2 * freq_cutoff - freq_bandwidth.value()) / sample_rate));
        W_cutoff.emplace_back((2 * freq_cutoff + freq_bandwidth.value()) / sample_rate);
      }
      else
      {
        W_cutoff.reserve(1);
        W_cutoff.emplace_back(2 * freq_cutoff / sample_rate);
      }
      
      double fs2 = 2.;
      for (auto& wc : W_cutoff)
        wc = 2. / fs2 * std::tan(math::c_pi * wc / fs2);

      auto lambda = std::pow(10., ripple / 20.);
      auto phi = std::log(lambda + std::sqrt(math::sq(lambda) - 1.)) / order;
      auto v = math::linspace<double>(0.5, 1, order - 0.5);
      auto theta = stlutils::mult_scalar(v, M_PI / order);
      auto st = stlutils::static_cast_vector<std::complex<double>>(
        stlutils::sin(theta));
      auto ct = stlutils::static_cast_vector<std::complex<double>>(
        stlutils::cos(theta));
      auto alpha = stlutils::mult_scalar(st, -std::sinh(phi));
      auto beta = stlutils::mult_scalar(ct, std::cosh(phi));
      
      FilterS s;
      auto num = stlutils::subtract(alpha, stlutils::mult_scalar(beta, 1i));
      auto den = stlutils::add(stlutils::comp_sq(alpha), stlutils::comp_sq(beta));
      s.poles = stlutils::comp_div(num, den);
      
      if (order % 2 == 0)
        s.zeroes = stlutils::scalar_div(1i, ct);
      else
      {
        // By removing the middle element, we avoid theta = pi/2 which would result in division by zero.
        auto ct2 = ct;
        int mid_idx = (static_cast<int>(ct2.size()) - 1) / 2;
        if (stlutils::erase_at(ct2, mid_idx))
          s.zeroes = stlutils::scalar_div(1i, ct2);
        else
        {
          std::cerr << "ERROR in create_ChebyshevII_filter() : Unable to calculate zeroes!" << std::endl;
          return flt;
        }
      }
      
      s.gain = std::abs(std::real(stlutils::prod(s.poles) / stlutils::prod(s.zeroes)));
      
      s = filter_edge_adjustment(s, type, W_cutoff[0], W_cutoff.size() == 2 ? W_cutoff[1] : 0.f);
      
      bilinear(s, fs2);
      
      flt.b = stlutils::static_cast_vector<float>(stlutils::mult_scalar(poly(s.zeroes), s.gain));
      flt.a = stlutils::static_cast_vector<float>(poly(s.poles));
      
      return flt;      
    }
    
    
    static void print_waveform_graph(const Waveform& wave, GraphType type,
                                     int width = 100, int height = 20,
                                     int a_idx_start = 0, std::optional<int> a_idx_end = std::nullopt)
    {
      int tot_buffer_len = static_cast<int>(wave.buffer.size());
      int idx_start = a_idx_start;
      int idx_end = tot_buffer_len - 1;
      if (a_idx_end.has_value())
        idx_end = a_idx_end.value();
      if (idx_end >= tot_buffer_len)
        idx_end = tot_buffer_len - 1;
      
      std::vector<float> buffer(wave.buffer.begin() + idx_start, wave.buffer.begin() + idx_end + 1);
      auto buffer_size = static_cast<int>(buffer.size());
      int step = buffer_size / width;
      
      float max_amplitude = 1e-7f;
      for (auto s : buffer)
        max_amplitude = std::max(max_amplitude, std::abs(s));
      
      for (int i = height - 1; i >= 0; --i)
      {
        for (int j = 0; j < width; ++j)
        {
          int start = j * step;
          int end = (j + 1) * step;
          
          // Calculate the average amplitude in the current segment
          float sum = 0.0f;
          float min_val = +std::numeric_limits<float>::infinity();
          float max_val = -std::numeric_limits<float>::infinity();
          for (int k = start; k < end && k < buffer_size; ++k)
          {
            float val = buffer[k];
            sum += val;
            min_val = std::min(min_val, val);
            max_val = std::max(max_val, val);
          }
          float avg_amplitude = sum / step;
          
          // Normalize amplitude to fit within the range [-1, 1]
          float normalized_amplitude = avg_amplitude / max_amplitude;
          float normalized_min = min_val / max_amplitude;
          float normalized_max = max_val / max_amplitude;
          
          // Determine if the current position should be printed
          bool shouldPrint = false;
          const float half_height = 0.5f*(height - 1);
          auto scale_ampl = [half_height](int i) { return i / half_height - 1.0f; };
          auto scaled_line_curr = scale_ampl(i);
          auto scaled_line_next = scale_ampl(i + 1);
          auto rate = 0.f;
          switch (type)
          {
            case GraphType::PLOT_THICK0:
              rate = 0.2f;
              break;
            case GraphType::PLOT_THICK1:
              rate = 1.f;
              break;
            case GraphType::PLOT_THICK2:
              rate = 1.5f;
              break;
            case GraphType::PLOT_THICK3:
              rate = 3.f;
              break;
            default:
              break;
          }
          switch (type)
          {
            case GraphType::PLOT_THIN:
              shouldPrint = scaled_line_curr <= normalized_amplitude && normalized_amplitude < scaled_line_next;
              break;
            case GraphType::PLOT_THICK0:
            case GraphType::PLOT_THICK1:
            case GraphType::PLOT_THICK2:
            case GraphType::PLOT_THICK3:
            {
              auto dydx = (scaled_line_next - scaled_line_curr) / step;
              auto dydx_abs = std::abs(dydx);
              auto a = normalized_max + rate*dydx_abs;
              auto b = normalized_min - rate*dydx_abs;
              shouldPrint = scaled_line_curr <= a && b < scaled_line_next;
              break;
            }
            case GraphType::FILLED_BOTTOM_UP:
              shouldPrint = normalized_amplitude >= scaled_line_curr;
              break;
            case GraphType::FILLED_FROM_T_AXIS:
              if (normalized_amplitude < 0)
                shouldPrint = normalized_amplitude <= scaled_line_curr && scaled_line_curr <= 0;
              else
                shouldPrint = 0 <= scaled_line_curr && scaled_line_curr <= normalized_amplitude;
              break;
          }
          
          if (shouldPrint)
            std::cout << "*";
          else
            std::cout << " ";
        }
        std::cout << std::endl;
      }
    }
    
    static void print_waveform_graph(const Waveform& wave, GraphType type,
                                     int width = 100, int height = 20,
                                     float t_start = 0.f, std::optional<float> t_end = std::nullopt)
    {
      float dt = 1.f/wave.sample_rate;
      auto tot_buffer_len = static_cast<int>(wave.duration / dt);
      auto idx_start = static_cast<int>(t_start / dt);
      int idx_end = tot_buffer_len - 1;
      if (t_end.has_value())
        idx_end = static_cast<int>(t_end.value() / dt);
      if (idx_end >= tot_buffer_len)
        idx_end = tot_buffer_len - 1;
      
      print_waveform_graph(wave, type, width, height, idx_start, idx_end);
    }
    
    static float calc_time_from_num_cycles(const Waveform& wave, float num_cycles)
    {
      float T = 1/wave.frequency;
      return num_cycles * T;
    }
    
    static float calc_dt(const Waveform& wave)
    {
      return 1.f/wave.sample_rate;
    }
    
    static float calc_duration(const Waveform& wave)
    {
      float dt = 1.f/wave.sample_rate;
      size_t num_samples = wave.buffer.size();
      return num_samples * dt;
    }
    
    static size_t calc_num_samples(float duration_s, int sample_rate)
    {
      return static_cast<size_t>(std::round(duration_s * sample_rate));
    }
    
  private:
    // Function to calculate the fundamental frequency after ring modulation
    static float calc_fundamental_frequency(float frequency_A, float frequency_B)
    {
      // Calculate the GCD of the frequencies
      float gcd_result = math::gcd(frequency_A, frequency_B);
      
      // The GCD represents the fundamental frequency
      return gcd_result;
    }
    
    static std::vector<std::complex<float>> fft_rec(const std::vector<std::complex<float>>& x)
    {
      auto N = x.size();
      
      if (N == 1)
        return x;
        
      auto Nh = static_cast<size_t>(N/2);
        
      std::vector<std::complex<float>> x_even(Nh), x_odd(Nh), y(N, 0.f);
      for (size_t i = 0; i < Nh; ++i)
      {
        x_even[i] = x[2*i];
        x_odd[i] = x[2*i + 1];
      }
      
      auto y_even = fft_rec(x_even);
      auto y_odd = fft_rec(x_odd);
      
      for (size_t m = 0; m < N; ++m)
      {
        auto m_alias = m % Nh;
        auto w_m = std::exp(-2.if * static_cast<float>(M_PI * m) / static_cast<float>(N));
        y[m] = y_even[m_alias] + w_m * y_odd[m_alias];
      }
      
      return y;
    }
    
    static std::vector<std::complex<float>> ifft_rec(const std::vector<std::complex<float>>& x)
    {
      auto N = x.size();
      
      if (N == 1)
        return x;
        
      auto Nh = static_cast<size_t>(N/2);
        
      std::vector<std::complex<float>> x_even(Nh), x_odd(Nh), y(N, 0.f);
      for (size_t i = 0; i < Nh; ++i)
      {
        x_even[i] = x[2*i];
        x_odd[i] = x[2*i + 1];
      }
      
      auto y_even = ifft_rec(x_even);
      auto y_odd = ifft_rec(x_odd);
      
      for (size_t m = 0; m < N; ++m)
      {
        auto m_alias = m % Nh;
        auto w_m = std::exp(2.if * static_cast<float>(M_PI * m) / static_cast<float>(N));
        y[m] = y_even[m_alias] + w_m * y_odd[m_alias];
        //y[m] = (y_even[m_alias] + w_m * y_odd[m_alias])/static_cast<float>(N);
        //y[m] = (y_even[m_alias] + w_m * y_odd[m_alias])/static_cast<float>(Nh);
      }
      
      return y;
    }
    
    // Assuming 'signal' is your time-domain signal before FFT or after IFFT.
    static void apply_window(Waveform& wave, WindowType type)
    {
      const size_t N = wave.buffer.size();
      
      switch (type)
      {
        case WindowType::HAMMING:
          for (size_t i = 0; i < N; ++i)
          {
            float window_val = 0.54f - 0.46f * std::cos(math::c_2pi * i / (N - 1));
            wave.buffer[i] *= window_val;
          }
          break;
        case WindowType::HANNING:
          for (size_t i = 0; i < N; ++i)
          {
            float window_val = 0.5f * (1.0f - std::cos(math::c_2pi * i / (N - 1)));
            wave.buffer[i] *= window_val;
          }
          break;
      }
    }
    
    // Inspired by flanger from https://github.com/abaga129/lib_dsp .
    static std::vector<float> flanger(const std::vector<float>& x, float delay_time, float lfo_freq, float feedback, float Fs)
    {
      size_t N = x.size();
      std::vector<float> y(N, 0);
      
      size_t D = std::round(delay_time*Fs);
      std::vector<float> xd(D + 1, 0);
      
      int q = 0;
      
      // Calculate fade in/out lengths (10% of delay time)
      size_t fade_length = static_cast<size_t>(0.1 * D);
      
      for (size_t n = 0; n < N; ++n)
      {
        float t = n/Fs;
        int d = static_cast<int>(std::round(0.5f*D*std::sin(math::c_2pi * lfo_freq * t)));
        int tap_idx = q + d;
        if (tap_idx < 0)
          tap_idx += D + 1;
        if (tap_idx >= D + 1)
          tap_idx -= D + 1;
        
        // Apply crossfade at the start and end of the buffer
        float fade_in = 1.0f;
        float fade_out = 1.0f;
        if (n < fade_length)
          fade_in = static_cast<float>(n) / fade_length;
        if (n > N - fade_length)
          fade_out = static_cast<float>(N - n) / fade_length;

        y[n] = (1.f - feedback) * x[n] * fade_in + feedback * xd[tap_idx] * fade_out;
        xd[q] = x[n];
        q--;
        if (q < 0)
          q = static_cast<int>(D);
      }
      
      return y;
    }
    
    static FilterS filter_edge_adjustment(const FilterS& s, FilterOpType type, double Wl, double Wh)
    {
      using namespace stlutils;
      auto Nz = static_cast<int>(s.zeroes.size());
      auto Np = static_cast<int>(s.poles.size());
      
      FilterS s_out;
      
      if (Np == 0)
      {
        std::cerr << "Must have at least one pole!" << std::endl;
        return s_out;
      }
      if (Nz > Np)
      {
        std::cerr << "Cannot have fewer poles than zeroes!" << std::endl;
        return s_out;
      }
      
      if (type == FilterOpType::HighPass || type == FilterOpType::BandStop)
      {
        if (Nz == 0)
          s_out.gain = s.gain * std::real(
                                          1. / prod(unary_minus(s.poles)));
        else if (Np == 0)
          s_out.gain = s.gain * std::real(
                                          prod(unary_minus(s.zeroes)));
        else
          s_out.gain = s.gain * std::real(
                                          prod(unary_minus(s.zeroes)) /
                                          prod(unary_minus(s.poles)));
      }
      
      switch (type)
      {
        case FilterOpType::NONE:
          break;
        case FilterOpType::LowPass:
          s_out.gain = s.gain * std::pow(1. / Wl, Nz - Np);
          s_out.zeroes = mult_scalar(s.zeroes, Wl);
          s_out.poles = mult_scalar(s.poles, Wl);
          break;
          
        case FilterOpType::HighPass:
          s_out.poles = scalar_div(Wl, s.poles);
          if (Nz == 0)
            s_out.zeroes.resize(Np, 0.);
          else
          {
            s_out.zeroes = scalar_div(Wl, s.zeroes);
            if (Np > Nz)
              s_out.zeroes.insert(s_out.zeroes.end(), Np - Nz, 0.);
          }
          break;
          
        case FilterOpType::BandPass:
        {
          auto W_diff = Wh - Wl;
          auto W_prod = Wl * Wh;
          
          s_out.gain = s.gain * std::pow(1. / W_diff, Nz - Np);
          
          auto q = mult_scalar(s.poles, W_diff / 2.);
          auto r = sqrt(subtract_scalar(comp_sq(q), W_prod));
          append(s_out.poles, subtract(q, r));
          append(s_out.poles, add(q, r));
          
          if (Nz == 0)
            s_out.zeroes.resize(Np, 0.);
          else
          {
            q = mult_scalar(s.zeroes, W_diff / 2.);
            r = sqrt(subtract_scalar(comp_sq(q), W_prod));
            append(s_out.zeroes, subtract(q, r));
            append(s_out.zeroes, add(q, r));
            if (Np > Nz)
              s_out.zeroes.insert(s_out.zeroes.end(), Np - Nz, 0.);
          }
          break;
        }
          
        case FilterOpType::BandStop:
        {
          auto W_diff = Wh - Wl;
          auto W_prod = Wl * Wh;
          auto q = scalar_div(W_diff / 2., s.poles);
          auto r = sqrt(subtract_scalar(comp_sq(q), W_prod));
          append(s_out.poles, subtract(q, r));
          append(s_out.poles, add(q, r));
          
          if (Nz == 0)
          {
            auto a = std::sqrt(-W_prod);
            for (int i = 0; i < Np; ++i)
            {
              s_out.zeroes.emplace_back(-a);
              s_out.zeroes.emplace_back(+a);
            }
          }
          else
          {
            q = scalar_div(W_diff / 2., s.zeroes);
            r = sqrt(subtract_scalar(comp_sq(q), W_prod));
            append(s_out.zeroes, subtract(q, r));
            append(s_out.zeroes, add(q, r));
            if (Np > Nz)
            {
              auto a = std::sqrt(-W_prod);
              for (int i = 0; i < Np - Nz; ++i)
              {
                s_out.zeroes.emplace_back(-a);
                s_out.zeroes.emplace_back(+a);
              }
            }
          }
          break;
        }
      }
      
      return s_out;
    }
    
    static void bilinear(FilterS& s, double T)
    {
      using namespace stlutils;
      auto Nz = static_cast<int>(s.zeroes.size());
      auto Np = static_cast<int>(s.poles.size());
      
      std::complex<double> a = { 2. / T };
      s.gain *= std::real(prod(scalar_subtract(a, s.zeroes)) / prod(scalar_subtract(a, s.poles)));
      
      if (Np == 0)
      {
        std::cerr << "Must have at least one pole!" << std::endl;
        return;
      }
      if (Nz > Np)
      {
        std::cerr << "Cannot have fewer poles than zeroes!" << std::endl;
        return;
      }
      
      for (auto& pole : s.poles)
        pole = (2. + pole * T) / (2. - pole * T);
      
      if (Nz == 0)
        s.zeroes.resize(Nz, -1.);
      else
      {
        for (auto& zero : s.zeroes)
          zero = (2. + zero * T) / (2. - zero * T);
        for (int i = 0; i < Np - Nz; ++i)
          s.zeroes.emplace_back(-1.);
      }
    }
    
    static std::vector<double> poly(const std::vector<std::complex<double>>& roots)
    {
      std::vector<std::complex<double>> y = { 1. };
      for (const auto& root : roots)
      {
        y.insert(y.begin(), 0.);  // Multiply by (z - root)
        for (int j = 0; j < y.size() - 1; ++j)
          y[j] -= root * y[j + 1];
      }
      std::reverse(std::begin(y), std::end(y));
      
      std::vector<double> coeffs;
      coeffs.reserve(y.size());
      for (const auto& e : y)
        coeffs.emplace_back(std::real(e));
      return coeffs;
    }
    
  };
  
}
