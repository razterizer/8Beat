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

  // FFT Spectrum.
  struct Spectrum
  {
    std::vector<std::complex<float>> buffer;
    float freq_start = 0.f;
    float freq_end = 0.f;
    
    void copy_properties(const Spectrum& spectrum)
    {
      this->freq_start = spectrum.freq_start;
      this->freq_end = spectrum.freq_end;
    }
  };

}
