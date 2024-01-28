//
//  ChipTuneEngine.h
//  8-Bit Audio Emulator Lib
//
//  Created by Rasmus Anthin on 2024-01-21.
//
#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>
#include "AudioSourceHandler.h"
#include "../Terminal Text Lib/Delay.h"
#include <thread>
#include <atomic>

namespace audio
{

  class ChipTuneEngine
  {
  public:
    ChipTuneEngine(AudioSourceHandler& audioHandler)
      : m_audioHandler(audioHandler)
    {
    }

    // Load tune from a text file with a specific format
    bool load_tune(const std::string& filePath)
    {
      std::ifstream file(filePath);
      if (!file.is_open())
      {
        std::cerr << "Error opening tune file: " << filePath << std::endl;
        return false;
      }

      std::string line;
      while (std::getline(file, line))
      {
        parse_line(line);
      }

      return true;
    }

    // Play the loaded tune
    void play_tune()
    {
      for (const auto& note : m_tune)
      {
        play_note(note);
        Delay::sleep(note.duration_ms*1000);
      }
    }
    
    // Play the loaded tune in a separate thread
    void play_tune_async()
    {
      // Use std::thread and std::atomic_flag to safely start and stop the thread
      stop_audio_thread = false;
      audio_thread = std::thread([this] {
          play_tune();
      });
    
      // Detach the audio thread, allowing it to run independently
      audio_thread.detach();
    }

    // Stop the audio playback thread
    void stop_tune_async()
    {
      stop_audio_thread = true;

      // Optionally, you can join the thread here if you want to wait for it to finish
      if (audio_thread.joinable())
      {
        audio_thread.join();
      }
    }

    // Wait for the audio playback thread to finish
    void wait_for_completion()
    {
      if (audio_thread.joinable())
      {
        audio_thread.join();
      }
    }

  private:
    struct Note
    {
      float frequency = 0.f;
      float duration_ms = 0.f;
    };

    AudioSourceHandler& m_audioHandler;
    std::vector<Note> m_tune;

    void parse_line(const std::string& line)
    {
      std::istringstream iss(line);
      float frequency, duration;
      if (iss >> frequency >> duration)
      {
        m_tune.push_back({ frequency, duration });
      }
      else
      {
        std::cerr << "Error parsing line: " << line << std::endl;
      }
    }

    void play_note(const Note& note)
    {
      // Create a waveform for the note
      WaveformData wd = m_audioHandler.generate_waveform(
        WaveformType::SINE_WAVE,
        note.duration_ms / 1000.f, note.frequency);

      // Create an audio source from the waveform
      AudioSource* noteSource = m_audioHandler.create_source_from_waveform(wd);

      // Play the source
      if (noteSource)
      {
        noteSource->play();
      }
    }
    
    std::thread audio_thread;
    std::atomic<bool> stop_audio_thread { false };
  };

}
