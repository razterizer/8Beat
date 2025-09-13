//
//  demo_4.cpp
//  8Beat
//
//  Created by Rasmus Anthin on 2024-03-17.
//

#include "AudioSourceHandler.h"
#include "WaveformHelper.h"
#include <Termin8or/input/Keyboard.h>
#include <Core/StringHelper.h>



int main(int argc, char** argv)
{
  t8::StreamKeyboard keyboard;

  std::cout << "Signal:\n";
  beat::Waveform wd_fft;
  wd_fft.buffer = { -4, 2, 1, -5 };
  for (const auto s : wd_fft.buffer)
    std::cout << s << std::endl;
  wd_fft.sample_rate = 10;
  std::cout << "Sample Rate (signal): " << wd_fft.sample_rate << std::endl;
  std::cout << str::rep_char('-', 20) << std::endl;

  std::cout << "FFT:\n";
  auto spectrum = beat::WaveformHelper::fft(wd_fft);
  for (const auto& z : spectrum.buffer)
    std::cout << z << std::endl;
  std::cout << "FFT Freq Start: " << spectrum.freq_start << std::endl;
  std::cout << "FFT Freq End:   " << spectrum.freq_end << std::endl;
  std::cout << str::rep_char('-', 20) << std::endl;
  
  std::cout << "IFFT:\n";
  auto wd_ifft = beat::WaveformHelper::ifft(spectrum);
  for (const auto s : wd_ifft.buffer)
    std::cout << s << std::endl;  
  std::cout << "Sample Rate (IFFT): " << wd_ifft.sample_rate << std::endl;
  std::cout << str::rep_char('-', 20) << std::endl;
  
  keyboard.pressAnyKey();
  
  return 0;
}
