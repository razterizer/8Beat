//
//  AudioSourceHandler.h
//  8Beat
//
//  Created by Rasmus Anthin on 2024-01-19.
//

#pragma once

#include "Waveform.h"

#include <Core/Math.h>
#include <Core/Rand.h>
#include <Core/Delay.h>
//#define USE_APPLAUDIO // To be set via the compiler flag -DUSE_APPLAUDIO instead.
#ifdef USE_APPLAUDIO
#include <AudioLibSwitcher/AudioLibSwitcher_applaudio.h>
#define SET_BUFFER_DATA m_audio_lib.set_buffer_data_32f
#define SAMPLE_TYPE float
#else
#include <AudioLibSwitcher/AudioLibSwitcher_OpenAL.h>
#define SET_BUFFER_DATA m_audio_lib.set_buffer_data_16s
#define SAMPLE_TYPE short
#endif

#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <numeric>
#include <variant>
#include <complex>
#include <optional>



namespace beat
{
  enum class PlaybackMode { NONE, SLEEP, STATE_WAIT };
  //const short c_amplitude_0 = 32767;
  const short c_short_min = -32768;
  const short c_short_max = +32767;
  const float c_short_lim_f = 32768.f;
  const float c_short_min_f = static_cast<float>(c_short_min) - 1e-6f;
  const float c_short_max_f = static_cast<float>(c_short_max) + 1e-6f;
#ifdef USE_APPLAUDIO
  audio::AudioLibSwitcher_applaudio m_audio_lib;
#else
  audio::AudioLibSwitcher_OpenAL m_audio_lib;
#endif
  
  class AudioSourceBase
  {
  protected:
    unsigned int m_sourceID = 0;
    unsigned int m_bufferID = 0;
    float m_duration_s = 0.f;
    
    inline short float_to_short(float val) noexcept
    {
      return static_cast<short>(std::clamp(c_short_lim_f * val, c_short_min_f, c_short_max_f));
    }
    
  public:
    AudioSourceBase()
    {
      m_sourceID = m_audio_lib.create_source();
      m_bufferID = m_audio_lib.create_buffer();
    }
    virtual ~AudioSourceBase()
    {
      m_audio_lib.destroy_buffer(m_bufferID);
      m_audio_lib.destroy_source(m_sourceID);
    }
  
    virtual void play(PlaybackMode playback_mode = PlaybackMode::NONE)
    {
      m_audio_lib.play_source(m_sourceID);
      
      switch (playback_mode)
      {
        case PlaybackMode::NONE:
          break;
        case PlaybackMode::SLEEP:
          Delay::sleep(static_cast<int>(m_duration_s*1e6f));
          break;
        case PlaybackMode::STATE_WAIT:
          do
          {
          } while (is_playing());
          break;
      }
    }
    
    bool is_playing() const
    {
      return m_audio_lib.is_source_playing(m_sourceID);
    }
    
    void pause()
    {
      m_audio_lib.pause_source(m_sourceID);
    }
    
    void stop()
    {
      m_audio_lib.stop_source(m_sourceID);
    }
    
    void set_volume(float volume)
    {
      m_audio_lib.set_source_volume(m_sourceID, volume);
    }
    
    void set_pitch(float pitch)
    {
      m_audio_lib.set_source_pitch(m_sourceID, pitch);
    }
    
    void set_looping(bool loop)
    {
      m_audio_lib.set_source_looping(m_sourceID, loop);
    }
    
    void detach()
    {
      m_audio_lib.detach_buffer_from_source(m_sourceID);
    }
  };
  
  class AudioSource : public AudioSourceBase
  {
  public:
    AudioSource() = default;
    AudioSource(const Waveform& wave_mono)
    {
      update_buffer(wave_mono);
    }
    
    AudioSource(const Waveform& wave_stereo_left, const Waveform& wave_stereo_right)
    {
      update_buffer(wave_stereo_left, wave_stereo_right);
    }
    
    bool update_buffer(const Waveform& wave_mono)
    {
      num_channels = 1;
      // Set source parameters (adjust as needed)
      m_audio_lib.set_source_standard_params(m_sourceID);
      
      m_duration_s = wave_mono.duration;
      
      // Load buffer data
#ifdef USE_APPLAUDIO
      m_buffer_i = wave_mono.buffer;
#else
      int N = stlutils::sizeI(wave_mono.buffer);
      m_buffer_i.resize(N, 0);
      for (int i = 0; i < N; ++i)
        m_buffer_i[i] = float_to_short(wave_mono.buffer[i]);
#endif

      m_audio_lib.destroy_buffer(m_bufferID);
      //if (m_bufferID == 0)
      m_bufferID = m_audio_lib.create_buffer();
      
      SET_BUFFER_DATA(m_bufferID, m_buffer_i, num_channels, wave_mono.sample_rate);
      
      // Attach buffer to source
      m_audio_lib.attach_buffer_to_source(m_sourceID, m_bufferID);
      
      // Check for errors
      auto error_msg = m_audio_lib.check_error();
      if (!error_msg.empty())
        std::cerr << "Error creating audio source: " << error_msg << std::endl;
      return true;
    }
    
    bool update_buffer(const Waveform& wave_stereo_left, const Waveform& wave_stereo_right)
    {
      num_channels = 2;
      // Set source parameters (adjust as needed)
      m_audio_lib.set_source_standard_params(m_sourceID);
      
      m_duration_s = std::max(wave_stereo_left.duration, wave_stereo_right.duration);
      auto N = std::max(stlutils::sizeI(wave_stereo_left.buffer), stlutils::sizeI(wave_stereo_right.buffer));
      int common_sample_rate = std::max(wave_stereo_left.sample_rate, wave_stereo_right.sample_rate);
      
      // Load buffer data
#ifdef USE_APPLAUDIO
      m_buffer_i.resize(num_channels * N, 0.f);
      for (int i = 0; i < N; ++i)
      {
        m_buffer_i[num_channels * i + 0] = stlutils::try_get(wave_stereo_left.buffer, i, 0.f);
        m_buffer_i[num_channels * i + 1] = stlutils::try_get(wave_stereo_right.buffer, i, 0.f);
      }
#else
      m_buffer_i.resize(num_channels * N, 0);
      for (int i = 0; i < N; ++i)
      {
        m_buffer_i[num_channels * i + 0] = float_to_short(stlutils::try_get(wave_stereo_left.buffer, i, 0.f));
        m_buffer_i[num_channels * i + 1] = float_to_short(stlutils::try_get(wave_stereo_right.buffer, i, 0.f));
      }
#endif

      m_audio_lib.destroy_buffer(m_bufferID);
      //if (m_bufferID == 0)
      m_bufferID = m_audio_lib.create_buffer();
      
      SET_BUFFER_DATA(m_bufferID, m_buffer_i, num_channels, common_sample_rate);
      
      // Attach buffer to source
      m_audio_lib.attach_buffer_to_source(m_sourceID, m_bufferID);
      
      // Check for errors
      auto error_msg = m_audio_lib.check_error();
      if (!error_msg.empty())
        std::cerr << "Error creating audio source: " << error_msg << std::endl;
      return true;
    }
    
  private:
    std::vector<SAMPLE_TYPE> m_buffer_i;
    int num_channels = 0;
  };
  
  struct AudioStreamListener
  {
    virtual ~AudioStreamListener() = default;
    virtual bool has_mono() const = 0;
    virtual bool has_stereo() const = 0;
    virtual float on_get_sample_mono(float /*t*/) const { return 0.f; };
    virtual std::pair<float, float> on_get_sample_stereo(float /*t*/) const { return { 0.f, 0.f }; }
  };
  
  class AudioStreamSource : public AudioSourceBase
  {
  public:
    AudioStreamSource(AudioStreamListener* listener, int sample_rate)
      : m_listener(listener)
      , m_sample_rate(sample_rate)
    {}
    
    bool update_buffer(int num_stream_samples, int requested_channels = 1, std::optional<float> pan = std::nullopt)
    {
      if (m_listener == nullptr)
        return false;
        
      if (requested_channels == 1 && m_listener->has_mono())
        num_channels = 1;
      else if (requested_channels == 2 && m_listener->has_stereo())
        num_channels = 2;
      else
        return false;
    
      m_buffer_i.resize(num_channels * num_stream_samples);
      
      float dt = 1.f/m_sample_rate;
      //std::cout << dt << std::endl;
      m_duration_s = num_stream_samples * dt;
      if (num_channels == 1)
      {
        for (int i = 0; i < num_stream_samples; ++i)
        {
          t = t_prev + i * dt;
#ifdef USE_APPLAUDIO
          m_buffer_i[i] = m_listener->on_get_sample_mono(t);
#else
          m_buffer_i[i] = float_to_short(m_listener->on_get_sample_mono(t));
#endif
        }
      }
      else if (num_channels == 2)
      {
        // #FIXME: Perhaps break out panning as a more low-level operation via AudioSourceBase function?
        auto pan_L = 1.f;
        auto pan_R = 1.f;
        if (pan.has_value())
        {
          pan_L = 1.f - pan.value();
          pan_R = pan.value();
        }
        for (int i = 0; i < num_stream_samples; ++i)
        {
          t = t_prev + i * dt;
          auto s = m_listener->on_get_sample_stereo(t);
#ifdef USE_APPLAUDIO
          m_buffer_i[num_channels * i + 0] = s.first * pan_L;
          m_buffer_i[num_channels * i + 1] = s.second * pan_R;
#else
          m_buffer_i[num_channels * i + 0] = float_to_short(s.first * pan_L);
          m_buffer_i[num_channels * i + 1] = float_to_short(s.second * pan_R);
#endif
        }
      }
      
      t = t_prev + m_duration_s;
      
      m_audio_lib.destroy_buffer(m_bufferID);
      //if (m_bufferID == 0)
      m_bufferID = m_audio_lib.create_buffer();
      
      SET_BUFFER_DATA(m_bufferID, m_buffer_i, num_channels, m_sample_rate);
      
      m_audio_lib.attach_buffer_to_source(m_sourceID, m_bufferID);
      
      t_prev = t;
      
      return true;
    }
    
    void reset_time()
    {
      t = 0.f;
      t_prev = 0.f;
    }
    
  private:
    AudioStreamListener* m_listener = nullptr;
    int m_sample_rate = 44100;
    std::vector<SAMPLE_TYPE> m_buffer_i;
    int num_channels = 0;
    float t = 0.f;
    float t_prev = 0.f;
  };
  
  
  class AudioSourceHandler
  {
  public:
    AudioSourceHandler(bool enable_audio = true)
    {
      m_audio_lib.init(enable_audio);
    }
    
    ~AudioSourceHandler()
    {
      m_audio_lib.finish();
    }
    
    AudioSource* create_source()
    {
      return m_sources.emplace_back(std::make_unique<AudioSource>()).get();
    }
    
    // Function to create a sound source with programmatically created buffer
    AudioSource* create_source_from_waveform(const Waveform& wave_mono)
    {
      return m_sources.emplace_back(std::make_unique<AudioSource>(wave_mono)).get();
    }
    
    // Function to create a sound source with programmatically created buffer
    AudioSource* create_source_from_waveform(const Waveform& wave_stereo_left, const Waveform& wave_stereo_right)
    {
      return m_sources.emplace_back(std::make_unique<AudioSource>(wave_stereo_left, wave_stereo_right)).get();
    }
    
    AudioStreamSource* create_stream_source(AudioStreamListener* listener, int sample_rate)
    {
      return m_stream_sources.emplace_back(
        std::make_unique<AudioStreamSource>(listener, sample_rate)).get();
    }
    
    void remove_source(AudioSource* source)
    {
      if (source == nullptr)
        return;
    
      auto it = std::remove_if(m_sources.begin(), m_sources.end(),
                               [source](const auto& ptr) { return ptr.get() == source; });
      
      if (it != m_sources.end())
      {
        // Stop playing the source
        source->stop();
        
        // Detach buffer from the source
        source->detach();
        
        // Erase the source from the vector
        m_sources.erase(it, m_sources.end());
      }
      else
      {
        // Handle error: Source not found
        std::cerr << "Error: Source not found for removal." << std::endl;
      }
    }
    
    void remove_source(AudioStreamSource* source)
    {
      if (source == nullptr)
        return;
        
      auto it = std::remove_if(m_stream_sources.begin(), m_stream_sources.end(),
                               [source](const auto& ptr) { return ptr.get() == source; });
      
      if (it != m_stream_sources.end())
      {
        // Stop playing the source
        source->stop();
        
        // Detach buffer from the source
        source->detach();
        
        // Erase the source from the vector
        m_stream_sources.erase(it, m_stream_sources.end());
      }
      else
      {
        // Handle error: Source not found
        std::cerr << "Error: Source not found for removal." << std::endl;
      }
    }
    
    // ///////////////////////////////////
    
  private:
    std::vector<std::unique_ptr<AudioSource>> m_sources;
    
    std::vector<std::unique_ptr<AudioStreamSource>> m_stream_sources;
  };
  
}
