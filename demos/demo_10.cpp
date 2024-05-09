//
//  demo_10.cpp
//  8Beat
//
//  Created by Rasmus Anthin on 2024-04-16.
//

#include "../AudioSourceHandler.h"
#include "../WaveformGeneration.h"
#include "../WaveformHelper.h"
#include "../../Termin8or/Keyboard.h"



int main(int argc, char** argv)
{
  enableRawMode();

  using namespace audio;
  AudioSourceHandler src_handler;
  WaveformGeneration wave_gen;
  
  // Arpeggio
  {
    std::cout << "Arpeggio" << std::endl;
    pressAnyKey();
    
    WaveformGenerationParams params;
    params.arpeggio.emplace_back(0.1f, 1.26f);
    params.arpeggio.emplace_back(0.2f, 1.5f);
    params.arpeggio.emplace_back(0.3f, 1.26f);
    params.arpeggio.emplace_back(0.4f, 1.f);
    params.arpeggio.emplace_back(0.5f, 1.26f);
    params.arpeggio.emplace_back(0.6f, 1.5f);
    params.arpeggio.emplace_back(0.7f, 1.26f);
    params.arpeggio.emplace_back(0.8f, 1.f);
    
    auto wd = wave_gen.generate_waveform(WaveformType::SQUARE, 1.f, 130.81f,
      params, 44100, false);
    wd = WaveformHelper::envelope_adsr(wd,
      Attack { ADSRMode::LOG, 50 },
      Decay { ADSRMode::EXP, 150 },
      Sustain { 0.7f },
      Release { ADSRMode::EXP, 70 });
    auto src = src_handler.create_source_from_waveform(wd);
    src->play(PlaybackMode::STATE_WAIT);
  }
  
  // Pickup/coin
  {
    std::cout << "Pickup / Coin" << std::endl;
    pressAnyKey();
  
    WaveformGenerationParams params;
    params.arpeggio.emplace_back(0.1f, 1.274f);
    params.duty_cycle = 0.5f;

    auto wd = wave_gen.generate_waveform(WaveformType::TRIANGLE, 0.5f, 1369.f,
      params, 44100, false);
    auto wd_adsr = WaveformHelper::envelope_adsr(wd,
      Attack { ADSRMode::LIN, 50, 0.f, 0.2f },
      Decay { ADSRMode::LIN, 100, 1.f},
      Sustain { 0.4f },
      Release { ADSRMode::EXP, 150 });
    auto src = src_handler.create_source_from_waveform(wd_adsr);
    src->play(PlaybackMode::STATE_WAIT);
  }
  
  // Laser/shoot
  {
    std::cout << "Laser / Shoot" << std::endl;
    pressAnyKey();
    
    WaveformGenerationParams params;
    params.freq_slide_vel = -20.49f;
    params.duty_cycle = 0.4762f;
    params.duty_cycle_sweep = -1.821f/100;

    auto wd = wave_gen.generate_waveform(WaveformType::SQUARE, 0.2f, 968.6f,
      params, 44100, false);
    auto wd_adsr = WaveformHelper::envelope_adsr(wd,
      Attack { ADSRMode::LIN, 0, 0.f, 0.2f },
      Decay { ADSRMode::LIN, 20, 1.f},
      Sustain { 0.4f },
      Release { ADSRMode::EXP, 13 });
    float delay_time = 3e-3f;
    float rate = 0.9f;
    float feedback = 0.9f;
    auto wd_flanger = WaveformHelper::flanger(wd_adsr, delay_time, rate, feedback);
    auto src = src_handler.create_source_from_waveform(wd_flanger);
    src->play(PlaybackMode::STATE_WAIT);
  }
  
  pressAnyKey();
  
  return 0;
}
