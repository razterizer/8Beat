//
//  WaveformHelper_Tests.h
//  8Beat
//
//  Created by Rasmus Anthin on 2024-09-29.
//

#pragma once
#include "../WaveformHelper.h"
#include <iostream>
#include <cassert>

namespace audio
{

  void unit_tests()
  {
    int N = 20;
    Waveform wave(N, 0.f);
    wave.frequency = 1.f/(N-1);
    wave.sample_rate = 1;
    
    auto k = math::c_2pi * wave.frequency / wave.sample_rate;
    for (int i = 0; i < N; ++i)
      wave.buffer[i] = std::sin(k * i);
      
    for (int i = 0; i < N; ++i)
      std::cout << wave.buffer[i] << std::endl;
      
    std::cout << std::endl;
      
    WaveformHelper::print_waveform_graph_idx(wave, GraphType::PLOT_THIN, N, 5);
  }

}
