//
//  FFTResult.h
//  8-Bit Audio Emulator Lib
//
//  Created by Rasmus Anthin on 2024-02-16.
//

#pragma once
#include <vector>
#include <complex>


namespace audio
{

  struct FFTResult
  {
    std::vector<std::complex<float>> spectrum;
    float freq_start = 0.f;
    float freq_end = 0.f;
  };

}
