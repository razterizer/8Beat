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
  
  // Vibrato
  {
    std::cout << "Vibrato" << std::endl;
    pressAnyKey();
    
    WaveformGenerationParams params;
    params.vibrato_depth = 0.3f;
    params.vibrato_freq = 2.f;
    params.vibrato_freq_vel = 1e-4f;
    params.vibrato_freq_acc = -1e-4f;
    params.duty_cycle = 0.5f;
    
    auto wd = wave_gen.generate_waveform(WaveformType::SQUARE, 2.5f, 130.81f,
      params, 44100, false);
    wd = WaveformHelper::envelope_adsr(wd,
      Attack { ADSRMode::LOG, 50 },
      Decay { ADSRMode::EXP, 150 },
      Sustain { 0.7f },
      Release { ADSRMode::EXP, 70 });
    auto src = src_handler.create_source_from_waveform(wd);
    src->play(PlaybackMode::STATE_WAIT);
  }
  
  // Noise @ Frequency
  {
    std::cout << "Noise @ Frequency" << std::endl;
    pressAnyKey();
    
    WaveformGenerationParams params;
    params.freq_slide_vel = 1.f;
    params.freq_slide_acc = -1.f;
    //params.noise_filter_order = 1;
    params.noise_filter_slot_dur_s = 16e-3f;
    
    auto wd = wave_gen.generate_waveform(WaveformType::NOISE, 2.f, 440.f, params, 44100, false);
    
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
  
  // Explosion
  {
    std::cout << "Explosion" << std::endl;
    pressAnyKey();
    
    WaveformGenerationParams params;
    params.freq_slide_vel = 2.f;
    params.freq_slide_acc = 5.f;
    params.vibrato_depth = 0.1421f;
    params.vibrato_freq = 10.16f;
    params.noise_filter_rel_bw = 0.1f;
    params.noise_filter_order = 1;
    params.noise_filter_slot_dur_s = 1e-3f;

    auto wd = wave_gen.generate_waveform(WaveformType::NOISE, 0.8f, 15.f,
      params, 44100, false);
      
    float delay_time = 1e-3f;
    float rate = 0.7f;
    float feedback = 0.4f;
    wd = WaveformHelper::flanger(wd, delay_time, rate, feedback);
    
    wd = WaveformHelper::envelope_adsr(wd,
      Attack { ADSRMode::LIN, 10, 0.f, 0.2f },
      Decay { ADSRMode::LIN, 50, 1.f },
      Sustain { 0.6f },
      Release { ADSRMode::LIN, 340 });
    
    WaveformHelper::normalize(wd);
    
    auto src = src_handler.create_source_from_waveform(wd);
    src->play(PlaybackMode::STATE_WAIT);
  }
  
  pressAnyKey();
  
  return 0;
}
