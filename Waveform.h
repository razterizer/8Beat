//
//  WaveformData.h
//  8-Bit Audio Emulator Lib
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
    std::vector<float> buffer;
    float frequency = 440.f; // A4
    float sample_rate = 44100.f;
    float duration = 5.f;
  };

}
