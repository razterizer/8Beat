//
//  demo_5.cpp
//  8Beat
//
//  Created by Rasmus Anthin on 2024-03-18.
//

#include "AudioSourceHandler.h"
#include "WaveformGeneration.h"
#include "WaveformHelper.h"
#include "ADSR.h"
#include <Termin8or/input/Keyboard.h>



int main(int argc, char** argv)
{
  t8::StreamKeyboard keyboard;
  
  beat::AudioSourceHandler src_handler;
  beat::WaveformGeneration wave_gen;
  
  int sample_rate = 44100;
  
  beat::WaveformGenerationParams params;

  auto wd = wave_gen.generate_waveform(beat::WaveformType::SQUARE, 3.f, 500.f,
                                       params, sample_rate, true);
  auto wd_adsr = beat::WaveformHelper::envelope_adsr(wd,
      { beat::ADSRMode::LOG, 300 }, { beat::ADSRMode::LOG, 500}, 0.4f, { beat::ADSRMode::LOG, 360 });
  beat::WaveformHelper::print_waveform_graph_t(wd_adsr, beat::GraphType::PLOT_THICK0, 100, 30, 0.f, std::nullopt);
  auto src_wd_adsr = src_handler.create_source_from_waveform(wd_adsr);
  src_wd_adsr->set_volume(0.5);
  src_wd_adsr->play(beat::PlaybackMode::STATE_WAIT);
  
  keyboard.pressAnyKey();
  
  wd_adsr = beat::WaveformHelper::envelope_adsr(wd,
      { beat::ADSRMode::LIN, 300, 0.2f, 0.5f }, { beat::ADSRMode::EXP, 500, 1.f }, 0.4f, { beat::ADSRMode::EXP, 360, 0.3f, 0.8f });
  beat::WaveformHelper::print_waveform_graph_t(wd_adsr, beat::GraphType::PLOT_THICK0, 100, 30, 0.f, std::nullopt);
  src_wd_adsr = src_handler.create_source_from_waveform(wd_adsr);
  src_wd_adsr->set_volume(0.5);
  src_wd_adsr->play(beat::PlaybackMode::STATE_WAIT);

  keyboard.pressAnyKey();
  
  return 0;
}
