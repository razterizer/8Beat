//
//  demo_4.cpp
//  8Beat
//
//  Created by Rasmus Anthin on 2024-03-17.
//

#include "../AudioSourceHandler.h"
#include "../WaveformHelper.h"
#include "../../Consolation/Keyboard.h"



int main(int argc, char** argv)
{
  enableRawMode();

  std::cout << "FFT:\n";
  audio::Waveform wd_fft;
  wd_fft.buffer = { -4, 2, 1, -5 };
  wd_fft.sample_rate = 10;
  auto spectrum = audio::WaveformHelper::fft(wd_fft);
  for (const auto& z : spectrum.buffer)
    std::cout << z << std::endl;
  std::cout << spectrum.freq_start << std::endl;
  std::cout << spectrum.freq_end << std::endl;
  
  std::cout << "IFFT:\n";
  auto wd_ifft = audio::WaveformHelper::ifft(spectrum);
  for (const auto& s : wd_ifft.buffer)
    std::cout << s << std::endl;
  std::cout << wd_ifft.sample_rate << std::endl;

#ifdef _MSC_VER
  std::cout << std::endl << "Press any key to exit." << std::endl;
  _getch();
#endif
  
  pressAnyKey();
  
  return 0;
}
