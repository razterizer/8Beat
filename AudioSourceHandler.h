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

  struct WaveformData
  {
    std::vector<short> waveform;
    float frequency = 440.f; // A4
    float sample_rate = 44100.f;
  };

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
    alBufferData(bufferID, AL_FORMAT_MONO16, wd.waveform.data(), static_cast<ALsizei>(wd.waveform.size() * sizeof(float)), wd.sample_rate);

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
  
  struct WaveformArgs
  {
    float duration = 5.f;
    float frequency = 440.f;
    const float amplitude = 32767.f;
    int buffer_len = 1000;
  };
  
  // Function to generate a simple waveform buffer
  WaveformData generate_waveform(
    const std::function<float(float, const WaveformArgs&)>& wave_func,
    float duration, float frequency, float sample_rate = 44100.f)
  {
    WaveformData wd;
    wd.frequency = frequency;
    wd.sample_rate = sample_rate;
    
    WaveformArgs args;
    args.duration = duration;
    args.frequency = frequency;
    args.buffer_len = duration * sample_rate;

    for (int i = 0; i < args.buffer_len; ++i)
    {
      float t = static_cast<float>(i) / sample_rate;
      float value = wave_func(t, args);
      wd.waveform.push_back(static_cast<short>(value));
    }

    return wd;
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

private:
  ALCdevice* m_device = nullptr;
  ALCcontext* m_context = nullptr;

  std::vector<std::unique_ptr<AudioSource>> m_sources;
};
