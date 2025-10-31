//
//  WaveformHelper_Tests.h
//  8Beat
//
//  Created by Rasmus Anthin on 2024-09-29.
//

#pragma once
#include "WaveformHelper.h"
#include <iostream>
#include <cassert>

namespace beat
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
    
    auto butter = WaveformHelper::create_Butterworth_filter(1,
                                                            FilterOpType::LowPass,
                                                            wave.frequency*0.9f, {},
                                                            wave.sample_rate);
    std::cout << "Filter Butterworth:" << std::endl;
    std::cout << "  a:" << std::endl;
    for (auto a : butter.a)
      std::cout << "    " << a << std::endl;
    std::cout << "  b:" << std::endl;
    for (auto b : butter.b)
      std::cout << "    " << b  << std::endl;
    
    assert(butter.a.size() == 2);
    assert(butter.a[0] == 1.f);
    assert(butter.a[1] - (-0.739251f) < 1e-6f);
    assert(butter.b.size() == 1);
    assert(butter.b[0] - (0.130375f) < 1e-6f);
  }

}
