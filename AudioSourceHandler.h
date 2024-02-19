//
//  AudioSourceHandler.h
//  8-Bit Audio Emulator Lib
//
//  Created by Rasmus Anthin on 2024-01-19.
//

#pragma once

#include "WaveformData.h"
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
  const float c_amplitude_0 = 32767.f;
  
  class AudioSource
  {
  public:
    AudioSource(ALuint sourceID, float dur_s)
      : m_sourceID(sourceID)
      , duration_s(dur_s)
    {
    }
    
    ~AudioSource()
    {
      // Clean up OpenAL source
      alDeleteSources(1, &m_sourceID);
    }
    
    void play(bool do_sleep = false)
    {
      alSourcePlay(m_sourceID);
      
      auto tol = 1e4f; //1e5f;
      if (do_sleep)
        Delay::sleep(duration_s*1e6f + tol);
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
    float duration_s = 0.f;
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
      m_sources.push_back(std::make_unique<AudioSource>(sourceID, wd.duration));
      
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
    
    // ///////////////////////////////////
    
  private:
    ALCdevice* m_device = nullptr;
    ALCcontext* m_context = nullptr;
    
    std::vector<std::unique_ptr<AudioSource>> m_sources;
  };
  
}
