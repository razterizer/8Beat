//
//  AudioSourceHandler.h
//  8-Bit Audio Emulator Lib
//
//  Created by Rasmus Anthin on 2024-01-19.
//

#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include "../../lib/Core Lib/Math.h"
#include "../../lib/Core Lib/Rand.h"
#include <iostream>
#include <numeric>
#include <variant>
#include <complex>

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
  enum class LowPassFilterType { NONE, Butterworth, ChebyshevTypeI, ChebyshevTypeII };
  
  const float c_amplitude_0 = 32767.f;
    
  // The returned data from the waveform generator.
  struct WaveformData
  {
    std::vector<float> buffer_f;
    float frequency = 440.f; // A4
    float sample_rate = 44100.f;
  };
  
  // Used inside the waveform lambda function.
  struct WaveformArgs
  {
    float duration = 5.f;
    float frequency_0 = 440.f;
    float frequency = frequency_0; // Set by freq_func().
    float amplitude = c_amplitude_0; // Set by ampl_func().
    int buffer_len = 1000;
    
    void init()
    {
      frequency = frequency_0;
      amplitude = c_amplitude_0;
    }
  };
  
  
  class AudioSource
  {
  public:
    AudioSource(ALuint sourceID)
    : m_sourceID(sourceID)
    {
    }
    
    ~AudioSource()
    {
      // Clean up OpenAL source
      alDeleteSources(1, &m_sourceID);
    }
    
    void play()
    {
      alSourcePlay(m_sourceID);
    }
    
    bool is_playing() const
    {
      ALint source_state = 0;
      alGetSourcei(m_sourceID, AL_SOURCE_STATE, &source_state);
      return source_state == AL_PLAYING;
    }
    
    void pause()
    {
      alSourcePause(m_sourceID);
    }
    
    void stop()
    {
      alSourceStop(m_sourceID);
    }
    
    void set_volume(float volume)
    {
      alSourcef(m_sourceID, AL_GAIN, volume);
    }
    
    void set_pitch(float pitch)
    {
      alSourcef(m_sourceID, AL_PITCH, pitch);
    }
    
    void set_looping(bool loop)
    {
      alSourcei(m_sourceID, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    }
    
    void detach()
    {
      alSourcei(m_sourceID, AL_BUFFER, 0);
    }
    
  private:
    ALuint m_sourceID = 0;
  };
  
  class AudioSourceHandler
  {
  public:
    AudioSourceHandler()
    {
      // Initialize OpenAL context and device
      m_device = alcOpenDevice(nullptr);
      if (m_device == nullptr)
      {
        // Handle error: Unable to open audio device
      }
      
      m_context = alcCreateContext(m_device, nullptr);
      if (m_context == nullptr)
      {
        // Handle error: Unable to create audio context
      }
      
      alcMakeContextCurrent(m_context);
    }
    
    ~AudioSourceHandler()
    {
      // Clean up OpenAL resources
      alcMakeContextCurrent(nullptr);
      alcDestroyContext(m_context);
      alcCloseDevice(m_device);
    }
    
    // Function to create a sound source from a file
    AudioSource* create_source_from_file(const std::string& wavFilePath)
    {
      ALuint sourceID = 0;
      
      // Generate OpenAL source
      alGenSources(1, &sourceID);
      
      // Load sound file
      std::ifstream file(wavFilePath, std::ios::binary);
      if (!file.is_open())
      {
        std::cerr << "Error opening sound file: " << wavFilePath << std::endl;
        return nullptr;
      }
      
      // Read the file contents into a buffer
      std::vector<char> fileBuffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
      
      // Generate OpenAL buffer
      ALuint bufferID = 0;
      alGenBuffers(1, &bufferID);
      
      // Load buffer data
      alBufferData(bufferID, AL_FORMAT_MONO16, fileBuffer.data(), static_cast<ALsizei>(fileBuffer.size()), 44100); // Adjust as needed
      
      // Attach buffer to source
      alSourcei(sourceID, AL_BUFFER, bufferID);
      
      // Check for errors
      ALenum error = alGetError();
      if (error != AL_NO_ERROR)
      {
        // Handle error
        std::cerr << "Error creating audio source from file: " << alGetString(error) << std::endl;
        return nullptr;
      }
      
      // Store the source in the vector
      m_sources.push_back(std::make_unique<AudioSource>(sourceID));
      
      return m_sources.back().get();
    }
    
    // Function to create a sound source with programmatically created buffer
    AudioSource* create_source_from_waveform(const WaveformData& wd)
    {
      ALuint sourceID = 0;
      
      // Generate OpenAL source
      alGenSources(1, &sourceID);
      
      // Set source parameters (adjust as needed)
      alSourcef(sourceID, AL_PITCH, 1.0f);
      alSourcef(sourceID, AL_GAIN, 1.0f);
      alSource3f(sourceID, AL_POSITION, 0.0f, 0.0f, 0.0f);
      alSource3f(sourceID, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
      alSourcei(sourceID, AL_LOOPING, AL_FALSE); // Adjust as needed
      
      // Generate OpenAL buffer
      ALuint bufferID = 0;
      alGenBuffers(1, &bufferID);
      
      // Load buffer data
      //alIsExtensionPresent("AL_EXT_float32");
      std::vector<short> buffer_i;
      buffer_i.resize(wd.buffer_f.size());
      int N = static_cast<int>(wd.buffer_f.size());
      for (int i = 0; i < N; ++i)
      {
        buffer_i[i] = static_cast<short>(c_amplitude_0 * wd.buffer_f[i]);
        buffer_i[i] = std::max<short>(-c_amplitude_0, buffer_i[i]);
        buffer_i[i] = std::min<short>(+c_amplitude_0, buffer_i[i]);
      }
      alBufferData(bufferID, AL_FORMAT_MONO16, buffer_i.data(), static_cast<ALsizei>(N * sizeof(float)), wd.sample_rate);
      
      // Attach buffer to source
      alSourcei(sourceID, AL_BUFFER, bufferID);
      
      // Check for errors
      ALenum error = alGetError();
      if (error != AL_NO_ERROR)
      {
        // Handle error
        std::cerr << "Error creating audio source: " << alGetString(error) << std::endl;
        return nullptr;
      }
      
      // Store the source in the vector
      m_sources.push_back(std::make_unique<AudioSource>(sourceID));
      
      return m_sources.back().get();
    }
    
    void remove_source(AudioSource* source)
    {
      auto it = std::remove_if(m_sources.begin(), m_sources.end(),
                               [source](const auto& ptr) { return ptr.get() == source; });
      
      if (it != m_sources.end())
      {
        // Stop playing the source
        (*it)->stop();
        
        // Detach buffer from the source
        (*it)->detach();
        
        // Erase the source from the vector
        m_sources.erase(it, m_sources.end());
      }
      else
      {
        // Handle error: Source not found
        std::cerr << "Error: Source not found for removal." << std::endl;
      }
    }
    
    using WaveformFunc = std::function<float(WAVEFORM_FUNC_ARGS)>;
    using FrequencyFunc = std::function<float(FREQUENCY_FUNC_ARGS)>;
    using AmplitudeFunc = std::function<float(AMPLITUDE_FUNC_ARGS)>;
    using WaveformFuncArg = std::variant<WaveformType, WaveformFunc>;
    using FrequencyFuncArg = std::variant<FrequencyType, FrequencyFunc>;
    using AmplitudeFuncArg = std::variant<AmplitudeType, AmplitudeFunc>;
    
    // Function to generate a simple waveform buffer
    WaveformData generate_waveform(const WaveformFuncArg& wave_func_arg = WaveformType::SINE_WAVE,
                                   float duration = 10.f, float frequency = 440.f,
                                   const FrequencyFuncArg& freq_func_arg = FrequencyType::CONSTANT,
                                   const AmplitudeFuncArg& ampl_func_arg = AmplitudeType::CONSTANT,
                                   float sample_rate = 44100.f,
                                   bool verbose = false)
    {
      WaveformData wd;
      wd.frequency = frequency;
      wd.sample_rate = sample_rate;
      
      WaveformArgs args;
      args.duration = duration;
      args.frequency_0 = frequency;
      args.buffer_len = duration * sample_rate;
      args.init();
      
      wd.buffer_f.resize(args.buffer_len);
      
      // Argument Functions
      auto wave_func = extract_waveform_func(wave_func_arg, verbose);
      auto freq_func = extract_frequency_func(freq_func_arg, verbose);
      auto ampl_func = extract_amplitude_func(ampl_func_arg, verbose);
      
      double accumulated_frequency = 0.0;
      
      for (int i = 0; i < args.buffer_len; ++i)
      {
        float t = static_cast<float>(i) / sample_rate;
        args.frequency = freq_func(t, duration, args.frequency_0);
        args.amplitude = ampl_func(t, duration);
        
        // Accumulate frequency for phase modulation
        accumulated_frequency += args.frequency;
        
        // Apply phase modulation similar to Octave code
        float phase_modulation = 2 * M_PI * accumulated_frequency / sample_rate;
        float sample = args.amplitude * wave_func(phase_modulation);
        wd.buffer_f[i] = sample;
      }
      
      return wd;
    }
    
    // Function to calculate the fundamental frequency after ring modulation
    int calc_fundamental_frequency(int frequency_A, int frequency_B)
    {
      // Calculate the GCD of the frequencies
      int gcd_result = math::gcd(frequency_A, frequency_B);
      
      // The GCD represents the fundamental frequency
      return gcd_result;
    }
    
    WaveformData ring_modulation(const WaveformData& wave_A, const WaveformData& wave_B)
    {
      // Resample both signals to a common sample rate
      float common_sample_rate = std::max(wave_A.sample_rate, wave_B.sample_rate);
      WaveformData resampled_A = resample(wave_A, common_sample_rate, LowPassFilterType::Butterworth);
      WaveformData resampled_B = resample(wave_B, common_sample_rate, LowPassFilterType::Butterworth);
      
      
      WaveformData prod;
      prod.sample_rate = common_sample_rate;
      prod.frequency = calc_fundamental_frequency(resampled_A.frequency, resampled_B.frequency);
      
      int Nmin = static_cast<int>(std::min(resampled_A.buffer_f.size(), resampled_B.buffer_f.size()));
      prod.buffer_f.resize(Nmin);
      
      for (int i = 0; i < Nmin; ++i)
      {
        float a = resampled_A.buffer_f[i];
        float b = resampled_B.buffer_f[i];
        prod.buffer_f[i] = a * b;
      }
      
      return prod;
    }
    
    WaveformData resample(const WaveformData& wave, float new_sample_rate,
      LowPassFilterType filter_type = LowPassFilterType::Butterworth,
      int filter_order = 1, float cutoff_freq_multiplier = 2.5f, float ripple = 0.1f)
    {
      if (wave.sample_rate == new_sample_rate)
        return wave;
    
      WaveformData resampled_wave;
      resampled_wave.frequency = wave.frequency;
      resampled_wave.sample_rate = new_sample_rate;
      
      // Calculate the resampling factor
      float resampling_factor = wave.sample_rate / new_sample_rate;
      
      // Calculate the new size of the buffer
      size_t new_size = static_cast<size_t>(wave.buffer_f.size() / resampling_factor);
      
      // Resize the buffer in the resampled_wave struct
      resampled_wave.buffer_f.resize(new_size);
      
      // Perform linear interpolation to fill the new buffer
      for (size_t i = 0; i < new_size; ++i)
      {
        float index_f = i * resampling_factor;
        size_t index = static_cast<size_t>(index_f);
        float fraction = index_f - index;
        
        // Linear interpolation
        resampled_wave.buffer_f[i] = (1.0f - fraction) * wave.buffer_f[index]
        + fraction * wave.buffer_f[index + 1];
      }
      
      float cutoff_frequency = cutoff_freq_multiplier * resampled_wave.frequency;
      
      // Apply the specified low-pass filter
      switch (filter_type)
      {
        case LowPassFilterType::NONE:
          break;
        case LowPassFilterType::Butterworth:
          apply_Butterworth_low_pass_filter(resampled_wave.buffer_f, cutoff_frequency, new_sample_rate, filter_order);
          break;
          
        case LowPassFilterType::ChebyshevTypeI:
          apply_ChebyshevI_low_pass_filter(resampled_wave.buffer_f, cutoff_frequency, new_sample_rate, ripple);
          break;
          
        case LowPassFilterType::ChebyshevTypeII:
          apply_ChebyshevII_low_pass_filter(resampled_wave.buffer_f, cutoff_frequency, new_sample_rate, ripple);
          break;
          
          // Add more cases for other filter types if needed
          
        default:
          // Handle unsupported filter types or provide a default behavior
          break;
      }
      
      return resampled_wave;
    }
    
    // Example of applying a Butterworth low-pass filter
    void apply_Butterworth_low_pass_filter(std::vector<float>& signal, float cutoff_frequency, float sample_rate, int filter_order)
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
    void apply_ChebyshevI_low_pass_filter(std::vector<float>& signal, float cutoff_frequency, float sample_rate, float ripple)
    {
      if (ripple <= 0.0)
      {
        // Invalid ripple value
        return;
      }
      
      // Calculate the analog cutoff frequency in radians
      //double omega_c = 2.0 * M_PI * cutoff_frequency / sample_rate;
      
      // Calculate the epsilon value for Chebyshev Type I filter
      //double epsilon = sqrt(1.0 / (pow(10, 0.1 * ripple)) - 1.0);
      double epsilon = 0;
      double epsilon_threshold = 1e-10; // Adjust the threshold as needed
      
      // Calculate the epsilon value for Chebyshev Type I filter
      double temp = std::pow(10, 0.1 * ripple) - 1.0;
      
      // Set epsilon to a small positive value to avoid NaN
      epsilon = (temp > epsilon_threshold) ? std::sqrt(temp) : 1e-5;
      
      // Calculate the poles of the Chebyshev Type I filter in the left half of the complex plane
      std::vector<std::complex<double>> poles;
      poles.emplace_back(-epsilon, 0.0);
      
      // Apply the Chebyshev Type I filter to the signal
      for (size_t n = 1; n < signal.size(); ++n)
      {
        double filtered_sample = epsilon * signal[n] + signal[n - 1];
        signal[n] = static_cast<float>(filtered_sample);
      }
    }
    
    // Example of applying a first-order Chebyshev low-pass filter (Type II)
    void apply_ChebyshevII_low_pass_filter(std::vector<float>& signal, float cutoff_frequency, float sample_rate, float ripple)
    {
      if (ripple <= 0.0)
      {
        // Invalid ripple value
        return;
      }
      
      // Calculate the analog cutoff frequency in radians
      //double omega_c = 2.0 * M_PI * cutoff_frequency / sample_rate;
      
      // Calculate the epsilon value for Chebyshev Type II filter
      //double epsilon = sqrt(1.0 / (pow(10, 0.1 * ripple)) - 1.0);
      double epsilon = 0;
      double epsilon_threshold = 1e-10; // Adjust the threshold as needed
      
      // Calculate the epsilon value for Chebyshev Type I filter
      double temp = std::pow(10, 0.1 * ripple) - 1.0;
      
      // Set epsilon to a small positive value to avoid NaN
      epsilon = (temp > epsilon_threshold) ? std::sqrt(temp) : 1e-5;
      
      // Initialize previous sample
      double y_prev = 0.0;
      
      // Apply the Chebyshev Type II filter to the signal
      for (size_t n = 1; n < signal.size(); ++n)
      {
        double filtered_sample = 2.0 * epsilon * y_prev - signal[n] + epsilon * signal[n];
        y_prev = signal[n];
        signal[n] = static_cast<float>(filtered_sample);
      }
    }
    
  private:
    ALCdevice* m_device = nullptr;
    ALCcontext* m_context = nullptr;
    
    std::vector<std::unique_ptr<AudioSource>> m_sources;
  
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
