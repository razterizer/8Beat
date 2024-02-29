//
//  AudioSourceHandler.h
//  8-Bit Audio Emulator Lib
//
//  Created by Rasmus Anthin on 2024-01-19.
//

#pragma once

#include "OpenALCallbacks.h"
#include "Waveform.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <vector>
#include <string>
#include <memory>
#include "../../lib/Core Lib/Math.h"
#include "../../lib/Core Lib/Rand.h"
#include "../../lib/Terminal Text Lib/Delay.h"
#include <iostream>
#include <numeric>
#include <variant>
#include <complex>
#include <optional>



namespace audio
{
  enum class PlaybackMode { NONE, SLEEP, STATE_WAIT };
  const float c_amplitude_0 = 32767.f;
  
  class AudioSource
  {
  public:
    AudioSource(const Waveform& wave)
      : m_duration_s(wave.duration)
    {
      // Generate OpenAL source
      alGenSources(1, &m_sourceID);
      
      // Set source parameters (adjust as needed)
      alSourcef(m_sourceID, AL_PITCH, 1.0f);
      alSourcef(m_sourceID, AL_GAIN, 1.0f);
      alSource3f(m_sourceID, AL_POSITION, 0.0f, 0.0f, 0.0f);
      alSource3f(m_sourceID, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
      alSourcei(m_sourceID, AL_LOOPING, AL_FALSE); // Adjust as needed
      
      // Generate OpenAL buffer
      alGenBuffers(1, &m_bufferID);
      
      // Load buffer data
      //alIsExtensionPresent("AL_EXT_float32");
      m_buffer_i.resize(wave.buffer.size());
      int N = static_cast<int>(wave.buffer.size());
      for (int i = 0; i < N; ++i)
      {
        m_buffer_i[i] = static_cast<short>(c_amplitude_0 * wave.buffer[i]);
        m_buffer_i[i] = std::max<short>(-c_amplitude_0, m_buffer_i[i]);
        m_buffer_i[i] = std::min<short>(+c_amplitude_0, m_buffer_i[i]);
      }
      
      alBufferData(m_bufferID, AL_FORMAT_MONO16, m_buffer_i.data(), static_cast<ALsizei>(N * sizeof(short)), wave.sample_rate);
      
      // Attach buffer to source
      alSourcei(m_sourceID, AL_BUFFER, m_bufferID);
      
      // Check for errors
      ALenum error = alGetError();
      if (error != AL_NO_ERROR)
      {
        // Handle error
        std::cerr << "Error creating audio source: " << alGetString(error) << std::endl;
      }
    }
    
    ~AudioSource()
    {
      // Clean up OpenAL source
      alDeleteSources(1, &m_sourceID);
      alDeleteBuffers(1, &m_bufferID);
    }
    
    void play(PlaybackMode playback_mode = PlaybackMode::NONE)
    {
      alSourcePlay(m_sourceID);
      
      switch (playback_mode)
      {
        case PlaybackMode::NONE:
          break;
        case PlaybackMode::SLEEP:
          Delay::sleep(m_duration_s*1e6f);
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
    ALuint m_bufferID = 0;
    float m_duration_s = 0.f;
    std::vector<short> m_buffer_i;
  };
  
  struct AudioStreamListener
  {
    virtual float on_get_sample(float /*t*/) const = 0;
  };
  
  class AudioStreamSource
  {
  public:
    AudioStreamSource(AudioStreamListener* listener, int sample_rate)
    : m_listener(listener),
    m_sample_rate(sample_rate)
    {
      alGenSources(1, &m_sourceID);
      alGenBuffers(1, &m_bufferID);
    }
    
    ~AudioStreamSource()
    {
      alDeleteSources(1, &m_sourceID);
      alDeleteBuffers(1, &m_bufferID);
    }
    
    void play(PlaybackMode playback_mode = PlaybackMode::NONE)
    {
      alSourcei(m_sourceID, AL_BUFFER, m_bufferID);
      alSourcePlay(m_sourceID);
      
      switch (playback_mode)
      {
        case PlaybackMode::NONE:
          break;
        case PlaybackMode::SLEEP:
          Delay::sleep(m_duration_s*1e6f);
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
      ALint state;
      alGetSourcei(m_sourceID, AL_SOURCE_STATE, &state);
      return state == AL_PLAYING;
    }
    
    void stop() {}
    void detach() {}
    
    void set_volume(float volume)
    {
      alSourcef(m_sourceID, AL_GAIN, volume);
    }
    
    void update_stream(int num_stream_samples)
    {
      m_buffer_i.resize(num_stream_samples);
      
      float dt = 1./m_sample_rate;
      float t = 0.f;
      for (int i = 0; i < num_stream_samples; ++i)
      {
        t = i * dt;
        m_buffer_i[i] = static_cast<short>(c_amplitude_0 * m_listener->on_get_sample(t));
        m_buffer_i[i] = std::max<short>(-c_amplitude_0, m_buffer_i[i]);
        m_buffer_i[i] = std::min<short>(+c_amplitude_0, m_buffer_i[i]);
      }
      
      m_duration_s = t;
      
      alBufferData(m_bufferID, AL_FORMAT_MONO16, m_buffer_i.data(), num_stream_samples * sizeof(short), m_sample_rate);
    }
    
  private:
    ALuint m_sourceID = 0;
    ALuint m_bufferID = 0;
    int m_sample_rate = 44100;
    float m_duration_s = 0.f;
    AudioStreamListener* m_listener = nullptr;
    std::vector<short> m_buffer_i;
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
        std::cerr << "ERROR: Unable to open audio device in AudioSourceHandler().\n";
      }
      
      m_context = alcCreateContext(m_device, nullptr);
      if (m_context == nullptr)
      {
        // Handle error: Unable to create audio context
        std::cerr << "ERROR: Unable to create audio context in AudioSourceHandler().\n";
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
    
    void remove_source(AudioSource* source)
    {
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
    ALCdevice* m_device = nullptr;
    ALCcontext* m_context = nullptr;
    
    std::vector<std::unique_ptr<AudioSource>> m_sources;
    
    std::vector<std::unique_ptr<AudioStreamSource>> m_stream_sources;
  };
  
}
