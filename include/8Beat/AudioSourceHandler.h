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
#define USE_APPLAUDIO
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
  const short c_amplitude_0 = 32767;
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
    AudioSource(const Waveform& wave)
    {
      // Set source parameters (adjust as needed)
      m_audio_lib.set_source_standard_params(m_sourceID);
      
      m_duration_s = wave.duration;
      
      // Load buffer data
#ifdef USE_APPLAUDIO
      m_buffer_i = wave.buffer;
#else
      m_buffer_i.resize(wave.buffer.size());
      int N = static_cast<int>(wave.buffer.size());
      for (int i = 0; i < N; ++i)
      {
        m_buffer_i[i] = static_cast<short>(c_amplitude_0 * wave.buffer[i]);
        m_buffer_i[i] = std::max<short>(-c_amplitude_0, m_buffer_i[i]);
        m_buffer_i[i] = std::min<short>(+c_amplitude_0, m_buffer_i[i]);
      }
#endif
      
      SET_BUFFER_DATA(m_bufferID, m_buffer_i, 1, wave.sample_rate);
      
      // Attach buffer to source
      m_audio_lib.attach_buffer_to_source(m_sourceID, m_bufferID);
      
      // Check for errors
      auto error_msg = m_audio_lib.check_error();
      if (!error_msg.empty())
        std::cerr << "Error creating audio source: " << error_msg << std::endl;
    }
    
  private:
    std::vector<SAMPLE_TYPE> m_buffer_i;
  };
  
  struct AudioStreamListener
  {
    virtual float on_get_sample(float /*t*/) const = 0;
  };
  
  class AudioStreamSource : public AudioSourceBase
  {
  public:
    AudioStreamSource() = default;
    AudioStreamSource(AudioStreamListener* listener, int sample_rate)
      : m_listener(listener)
      , m_sample_rate(sample_rate)
    {}
    
    virtual void play(PlaybackMode playback_mode = PlaybackMode::NONE) override
    {
      m_audio_lib.attach_buffer_to_source(m_sourceID, m_bufferID);
      AudioSourceBase::play(playback_mode);
    }
    
    void update_buffer(int num_stream_samples)
    {
      if (m_listener == nullptr)
        return;
    
      m_buffer_i.resize(num_stream_samples);
      
      float dt = 1.f/m_sample_rate;
      float t = 0.f;
      for (int i = 0; i < num_stream_samples; ++i)
      {
        t = i * dt;
#ifdef USE_APPLAUDIO
        m_buffer_i[i] = m_listener->on_get_sample(t);
#else
        m_buffer_i[i] = static_cast<short>(c_amplitude_0 * m_listener->on_get_sample(t));
        m_buffer_i[i] = std::max<short>(-c_amplitude_0, m_buffer_i[i]);
        m_buffer_i[i] = std::min<short>(+c_amplitude_0, m_buffer_i[i]);
#endif
      }
      
      m_duration_s = t;
      
      m_audio_lib.destroy_buffer(m_bufferID);
      m_bufferID = m_audio_lib.create_buffer();
      
      SET_BUFFER_DATA(m_bufferID, m_buffer_i, 1, m_sample_rate);
    }
    
    void update_buffer(const Waveform& wave)
    {
      auto Ns = static_cast<int>(wave.buffer.size());
      m_buffer_i.resize(Ns);
      
#ifdef USE_APPLAUDIO
      m_buffer_i = wave.buffer;
#else
      for (int i = 0; i < Ns; ++i)
      {
        m_buffer_i[i] = static_cast<short>(c_amplitude_0 * wave.buffer[i]);
        m_buffer_i[i] = std::max<short>(-c_amplitude_0, m_buffer_i[i]);
        m_buffer_i[i] = std::min<short>(+c_amplitude_0, m_buffer_i[i]);
      }
#endif
      
      m_duration_s = wave.duration;
      
      m_audio_lib.destroy_buffer(m_bufferID);
      m_bufferID = m_audio_lib.create_buffer();
      
      SET_BUFFER_DATA(m_bufferID, m_buffer_i, 1, m_sample_rate);
    }
    
  private:
    AudioStreamListener* m_listener = nullptr;
    int m_sample_rate = 44100;
    std::vector<SAMPLE_TYPE> m_buffer_i;
  };
  
  
  class AudioSourceHandler
  {
  public:
    AudioSourceHandler()
    {
      m_audio_lib.init();
    }
    
    ~AudioSourceHandler()
    {
      m_audio_lib.finish();
    }
    
    // Function to create a sound source with programmatically created buffer
    AudioSource* create_source_from_waveform(const Waveform& wave)
    {
      return m_sources.emplace_back(std::make_unique<AudioSource>(wave)).get();
    }
    
    AudioStreamSource* create_stream_source(AudioStreamListener* listener, int sample_rate)
    {
      return m_stream_sources.emplace_back(
        std::make_unique<AudioStreamSource>(listener, sample_rate)).get();
    }
    
    AudioStreamSource* create_stream_source()
    {
      return m_stream_sources.emplace_back(std::make_unique<AudioStreamSource>()).get();
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
