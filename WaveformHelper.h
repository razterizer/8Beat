//
//  WaveformHelper.h
//  8-Bit Audio Emulator Lib
//
//  Created by Rasmus Anthin on 2024-02-10.
//

#pragma once
#include "WaveformData.h"
#include "FFTResult.h"
#include "../../lib/Core Lib/MachineLearning/ann_cnn.h"
#include <complex>

using namespace std::complex_literals;


namespace audio
{
  
  enum class LowPassFilterType { NONE, Butterworth, ChebyshevTypeI, ChebyshevTypeII };
  enum class GraphType { PLOT_THIN, PLOT_THICK0, PLOT_THICK1, PLOT_THICK2, PLOT_THICK3, FILLED_BOTTOM_UP, FILLED_FROM_T_AXIS };
  enum class Complex2Real { ABS, REAL, IMAG };
  enum class WindowType { HAMMING, HANNING };
  
  class WaveformHelper
  {
  public:
    static Waveform ring_modulation(const Waveform& wave_A, const Waveform& wave_B)
    {
      // Resample both signals to a common sample rate
      float common_sample_rate = std::max(wave_A.sample_rate, wave_B.sample_rate);
      Waveform res_A = resample(wave_A, common_sample_rate, LowPassFilterType::Butterworth);
      Waveform res_B = resample(wave_B, common_sample_rate, LowPassFilterType::Butterworth);
      
      
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
      float common_sample_rate = std::max(wave.sample_rate, kernel.sample_rate);
      Waveform res_wave = resample(wave, common_sample_rate, LowPassFilterType::Butterworth);
      Waveform res_kernel = resample(kernel, common_sample_rate, LowPassFilterType::Butterworth);
      
      Waveform conv;
      conv.sample_rate = common_sample_rate;
      conv.frequency = calc_fundamental_frequency(res_wave.frequency, res_kernel.frequency);
      
      conv.buffer = ml::ann::conv_1d(res_wave.buffer, res_kernel.buffer, 0.f,
                                       ml::ann::ConvRange::Full, ml::ann::ConvType::Convolution);
                                       
      normalize_over(conv);
      //auto [minval, maxval] = WaveformHelper::find_min_max(conv, true);
      //std::cout << "minval = " << minval << std::endl;
      //std::cout << "maxval = " << maxval << std::endl;
      
      conv.duration = calc_duration(conv);
      
      return conv;
    }
    
    static Waveform reverb_fast(const Waveform& wave, const Waveform& kernel)
    {
      // Resample both signals to a common sample rate.
      float common_sample_rate = std::max(wave.sample_rate, kernel.sample_rate);
      Waveform res_wave = resample(wave, common_sample_rate, LowPassFilterType::Butterworth);
      Waveform res_kernel = resample(kernel, common_sample_rate, LowPassFilterType::Butterworth);
      
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
      prod.freq_start = spec_res_wave.freq_start;
      prod.freq_end = spec_res_wave.freq_end;
      
      auto reverb = ifft(prod);
      auto conv_full_size = Nw + Nk - 1;
      //reverb.buffer.resize(std::min(reverb.buffer.size(), res_kernel.buffer.size()));
      //reverb.buffer.resize(std::min(reverb.buffer.size(), res_wave.buffer.size()));
      reverb.buffer.resize(conv_full_size);
      reverb.sample_rate = common_sample_rate;
      reverb.frequency = calc_fundamental_frequency(res_wave.frequency, res_kernel.frequency);
      reverb.duration = calc_duration(reverb);
      
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
      int N = std::pow<int>(2, std::ceil(std::log(sz)/std::log(2)));
    
      std::vector<std::complex<float>> input(std::begin(wave.buffer), std::end(wave.buffer));
      // Padding.
      for (int i = 0; i < N - sz; ++i)
        input.emplace_back(0);
      
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
      int N = std::pow<int>(2, std::ceil(std::log(sz)/std::log(2)));
    
      std::vector<std::complex<float>> input(std::begin(spectrum.buffer), std::end(spectrum.buffer));
      // Padding.
      for (int i = 0; i < N - sz; ++i)
        input.emplace_back(0);
      
      auto output = ifft_rec(input);
      
      // Normalize the output.
      for (auto& s : output)
        s /= static_cast<float>(N);
      
      Waveform result;
      result.buffer = complex2real(output, c2r_filter);
      // Calculate frequency axis.
      result.sample_rate = spectrum.freq_end * 2.f;
      
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
          s /= max_val * amplitude_limit;
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
    static void scale(Waveform& wd, float scale = 1.f)
    {
      auto [_, max_val] = find_min_max(wd, true);
      for (auto& s : wd.buffer)
        s /= max_val * scale;
    }
    
    static Waveform resample(const Waveform& wave, float new_sample_rate = 44100.f,
                                 LowPassFilterType filter_type = LowPassFilterType::Butterworth,
                                 int filter_order = 1, float cutoff_freq_multiplier = 2.5f, float ripple = 0.1f)
    {
      if (wave.sample_rate == new_sample_rate)
        return wave;
      
      Waveform resampled_wave;
      resampled_wave.frequency = wave.frequency;
      resampled_wave.sample_rate = new_sample_rate;
      resampled_wave.duration = wave.duration;
      
      // Calculate the resampling factor
      float resampling_factor = wave.sample_rate / new_sample_rate;
      
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
      
      float cutoff_frequency = cutoff_freq_multiplier * resampled_wave.frequency;
      
      // Apply the specified low-pass filter
      switch (filter_type)
      {
        case LowPassFilterType::NONE:
          break;
        case LowPassFilterType::Butterworth:
          apply_Butterworth_low_pass_filter(resampled_wave.buffer, filter_order, cutoff_frequency, new_sample_rate);
          break;
          
        case LowPassFilterType::ChebyshevTypeI:
          apply_ChebyshevI_low_pass_filter(resampled_wave.buffer, filter_order, cutoff_frequency, new_sample_rate, ripple);
          break;
          
        case LowPassFilterType::ChebyshevTypeII:
          apply_ChebyshevII_low_pass_filter(resampled_wave.buffer, filter_order, cutoff_frequency, new_sample_rate, ripple);
          break;
          
          // Add more cases for other filter types if needed
          
        default:
          // Handle unsupported filter types or provide a default behavior
          break;
      }
      
      return resampled_wave;
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
      float dt = 1/wave.sample_rate;
      int tot_buffer_len = wave.duration / dt;
      int idx_start = t_start / dt;
      int idx_end = tot_buffer_len - 1;
      if (t_end.has_value())
        idx_end = t_end.value() / dt;
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
      return 1/wave.sample_rate;
    }
    
    static float calc_duration(const Waveform& wave)
    {
      float dt = 1/wave.sample_rate;
      size_t num_samples = wave.buffer.size();
      return num_samples * dt;
    }
    
  private:
    // Function to calculate the fundamental frequency after ring modulation
    static int calc_fundamental_frequency(int frequency_A, int frequency_B)
    {
      // Calculate the GCD of the frequencies
      int gcd_result = math::gcd(frequency_A, frequency_B);
      
      // The GCD represents the fundamental frequency
      return gcd_result;
    }
    
    // Example of applying a Butterworth low-pass filter
    static void apply_Butterworth_low_pass_filter(std::vector<float>& signal, int filter_order,
                                                  float cutoff_frequency, float sample_rate)
    {
      if (filter_order <= 0)
      {
        // Invalid filter order
        return;
      }
      
      // Calculate the analog cutoff frequency in radians
      double omega_c = 2.0 * M_PI * cutoff_frequency / sample_rate;
      
      // Calculate poles of the Butterworth filter in the left half of the complex plane
      std::vector<std::complex<double>> poles;
      for (int k = 0; k < filter_order; ++k)
      {
        double real_part = -std::sin(M_PI * (2.0 * k + 1) / (2.0 * filter_order));
        double imag_part = std::cos(M_PI * (2.0 * k + 1) / (2.0 * filter_order));
        poles.emplace_back(real_part, imag_part);
      }
      
      // Apply the Butterworth filter to the signal
      for (size_t n = filter_order; n < signal.size(); ++n)
      {
        double filtered_sample = 0.0;
        for (int k = 0; k < filter_order; ++k)
        {
          // Calculate the z-transform of the filter transfer function
          std::complex<double> z = std::exp(std::complex<double>(0, omega_c)) * poles[k];
          filtered_sample += real(z) * signal[n - k];
        }
        
        signal[n] = static_cast<float>(filtered_sample);
      }
    }
    
    // Example of applying a first-order Chebyshev low-pass filter (Type I)
    static void apply_ChebyshevI_low_pass_filter(std::vector<float>& signal, int filter_order,
                                                 float cutoff_frequency, float sample_rate, float ripple)
    {
      if (ripple <= 0.0)
      {
        // Invalid ripple value
        return;
      }
      
      // Calculate the epsilon value for Chebyshev Type I filter
      double temp = pow(10, 0.1 * ripple) - 1.0;
      double epsilon = (temp >= 0.0) ? sqrt(temp) : 0.0;
      
      // Calculate the poles of the Chebyshev Type I filter in the left half of the complex plane
      std::vector<std::complex<double>> poles;
      for (int k = 0; k < filter_order; ++k)
      {
        double angle = M_PI * (2 * k + 1) / (2.0 * filter_order);
        poles.emplace_back(-epsilon * sin(angle), epsilon * cos(angle));
      }
      
      // Apply the Chebyshev Type I filter to the signal
      for (size_t n = filter_order; n < signal.size(); ++n)
      {
        double filtered_sample = 0.0;
        for (int k = 0; k < filter_order; ++k)
        {
          filtered_sample += poles[k].real() * signal[n - k];
        }
        signal[n] = static_cast<float>(filtered_sample);
      }
    }
    
    // Example of applying a first-order Chebyshev low-pass filter (Type II)
    static void apply_ChebyshevII_low_pass_filter(std::vector<float>& signal, int filter_order,
                                                  float cutoff_frequency, float sample_rate, float ripple)
    {
      if (ripple <= 0.0)
      {
        // Invalid ripple value
        return;
      }
      
      // Calculate the epsilon value for Chebyshev Type II filter
      double temp = pow(10, 0.1 * ripple) - 1.0;
      double epsilon = (temp >= 0.0) ? sqrt(temp) : 0.0;
      
      // Calculate the poles of the Chebyshev Type II filter in the left half of the complex plane
      std::vector<std::complex<double>> poles;
      for (int k = 0; k < filter_order; ++k)
      {
        double angle = M_PI * (2 * k + 1) / (2.0 * filter_order);
        poles.emplace_back(-epsilon * sin(angle), epsilon * cos(angle));
      }
      
      // Apply the Chebyshev Type II filter to the signal
      for (size_t n = filter_order; n < signal.size(); ++n)
      {
        double filtered_sample = 0.0;
        for (int k = 0; k < filter_order; ++k)
        {
          filtered_sample += poles[k].real() * signal[n - k];
        }
        signal[n] = static_cast<float>(filtered_sample);
      }
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
            float window_val = 0.54f - 0.46f * std::cos(2.0f * M_PI * i / (N - 1));
            wave.buffer[i] *= window_val;
          }
          break;
        case WindowType::HANNING:
          for (size_t i = 0; i < N; ++i)
          {
            float window_val = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (N - 1)));
            wave.buffer[i] *= window_val;
          }
          break;
      }
    }
    
  };
  
}