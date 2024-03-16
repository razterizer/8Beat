//
//  WaveformData.h
//  8Beat
//
//  Created by Rasmus Anthin on 2024-02-11.
//

#pragma once

#include <vector>


namespace audio
{

  // The returned data from the waveform generator or file loader.
  struct Waveform
  {
    Waveform() = default;
    // Initialize the buffer with DC value.
    Waveform(size_t N, float dc_val)
     : buffer(N, dc_val)
    {}
  
    std::vector<float> buffer;
    float frequency = 440.f; // A4
    float sample_rate = 44100.f;
    float duration = 5.f;
    
    void copy_properties(const Waveform& wave)
    {
      this->frequency = wave.frequency;
      this->sample_rate = wave.sample_rate;
      this->duration = wave.duration;
    }
    
    void update_duration()
    {
      float dt = 1/sample_rate;
      size_t num_samples = buffer.size();
      this->duration = num_samples * dt;
    }
  };

}
