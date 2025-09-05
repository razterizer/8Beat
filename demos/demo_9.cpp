//
//  demo_9.cpp
//  8Beat
//
//  Created by Rasmus Anthin on 2024-04-08.
//

#include "../AudioSourceHandler.h"
#include "../WaveformGeneration.h"
#include "../WaveformHelper.h"
#include <Termin8or/Keyboard.h>



int main(int argc, char** argv)
{
  t8::input::StreamKeyboard keyboard;
  
  audio::AudioSourceHandler src_handler;
  audio::WaveformGeneration wave_gen;
  
  //auto wd = wave_gen.generate_waveform(audio::WaveformType::TRIANGLE, 3.f, 385.4f);
  auto wd = wave_gen.generate_waveform(audio::WaveformType::SAWTOOTH, 3.f, 440.f);
  float delay_time = 3e-3f;
  float rate = 0.9f;
  //float delay_time_sweep = 0.01f;
  float feedback = 0.9f;
  auto wd_flanger = audio::WaveformHelper::flanger(wd, delay_time, rate, feedback);
  auto src = src_handler.create_source_from_waveform(wd_flanger);
  src->play(audio::PlaybackMode::STATE_WAIT);
  
  keyboard.pressAnyKey();
  
  return 0;
}
