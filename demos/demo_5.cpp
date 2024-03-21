//
//  demo_5.cpp
//  8Beat
//
//  Created by Rasmus Anthin on 2024-03-18.
//

#include "../AudioSourceHandler.h"
#include "../WaveformGeneration.h"
#include "../WaveformHelper.h"
#include "../ADSR.h"
#include "../../Consolation/Keyboard.h"



int main(int argc, char** argv)
{
  enableRawMode();
  
  audio::AudioSourceHandler src_handler;
  audio::WaveformGeneration wave_gen;
  
  float sample_rate = 441100.f; // #FIXME: Convert to int.

  auto wd = wave_gen.generate_waveform(audio::WaveformType::SQUARE_WAVE, 3.f, 500.f,
                                      audio::FrequencyType::CONSTANT, audio::AmplitudeType::CONSTANT, audio::PhaseType::ZERO, sample_rate);
  auto wd_adsr = audio::WaveformHelper::envelope_adsr(wd,
      { audio::ADSRMode::LOG, 300 }, { audio::ADSRMode::LOG, 500}, 0.4f, { audio::ADSRMode::LOG, 360 });
  audio::WaveformHelper::print_waveform_graph(wd_adsr, audio::GraphType::PLOT_THICK0, 100, 30, 0.f, std::nullopt);
  auto src_wd_adsr = src_handler.create_source_from_waveform(wd_adsr);
  src_wd_adsr->set_volume(0.5);
  src_wd_adsr->play(audio::PlaybackMode::STATE_WAIT);

  pressAnyKey();
  
  return 0;
}
