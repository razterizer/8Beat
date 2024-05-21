//
//  demo_10.cpp
//  8Beat
//
//  Created by Rasmus Anthin on 2024-04-16.
//

#include "../AudioSourceHandler.h"
#include "../WaveformGeneration.h"
#include "../WaveformHelper.h"
#include "../SFX.h"
#include "../../Termin8or/Keyboard.h"



int main(int argc, char** argv)
{
  enableRawMode();

  using namespace audio;
  AudioSourceHandler src_handler;
  WaveformGeneration wave_gen;
  rnd::srand_time();
  
  #if 1
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
  #endif
  
  #if 1
  // Pickup/coin
  {
    std::cout << "Pickup / Coin" << std::endl;
    pressAnyKey();

    for (int i = 0; i < 10; ++i)
    {
      std::vector<float> vp
      {
        rnd::rand_float(-0.8f, 2.f),
        rnd::rand_float(-0.9f, 2.f),
        rnd::rand_float(-1.f, 1.f),
        rnd::rand_float(-0.f, 1.2f),
        rnd::rand_float(-0.8f, 2.f),
        rnd::rand_float(-1.f, .7f),
        rnd::rand_float(-.95f, 1.5f),
        rnd::rand_float(-1.f, 1.f),
        rnd::rand_float(-1.f, 4.f),
        rnd::rand_float(-0.8, 1.5f)
      };
      if (i == 0) // First show the default sound.
       vp.clear();
      auto wd = SFX::generate(SFXType::COIN, vp, false);
      auto src = src_handler.create_source_from_waveform(wd);
      src->play(PlaybackMode::STATE_WAIT);
    }
  }
  #endif
  
  #if 1
  // Laser/shoot
  {
    std::cout << "Laser / Shoot" << std::endl;
    pressAnyKey();
    
    for (int i = 0; i < 10; ++i)
    {
      std::vector<float> vp
      {
        rnd::rand_float(-1.f, 2.f),
        rnd::rand_float(-1.f, 1.1f),
        rnd::rand_float(-1.f, 1.5f),
        rnd::rand_float(-0.95f, 5.f),
        rnd::rand_float(-0.9, 2.f),
        rnd::rand_float(-1.f, 2.f),
        rnd::rand_float(-1.f, 20.f),
        rnd::rand_float(-1.f, 4.f),
        rnd::rand_float(-0.8f, 1.5f),
        rnd::rand_float(-0.95f, 2.f),
        rnd::rand_float(-0.9f, 1.f/9.f),
        rnd::rand_float(-0.9f, 1.f/9.f)
      };
      if (i == 0) // First show the default sound.
       vp.clear();
      auto wd = SFX::generate(SFXType::LASER, vp, false);
      auto src = src_handler.create_source_from_waveform(wd);
      src->play(PlaybackMode::STATE_WAIT);
    }
  }
  #endif
  
  #if 1
  // Explosion
  {
    std::cout << "Explosion" << std::endl;
    pressAnyKey();
    
    for (int i = 0; i < 10; ++i)
    {
      std::vector<float> vp
      {
        rnd::rand_float(-3.f, 2.5f),
        rnd::rand_float(-3.f, 2.f),
        rnd::rand_float(-1.f, 5.f),
        rnd::rand_float(-1.f, 3.f),
        rnd::rand_float(-0.8f, 4.f),
        static_cast<float>(rnd::rand_int(1, 3)),
        rnd::rand_float(-0.9f, 1.f),
        rnd::rand_float(-0.9f, 3.f),
        rnd::rand_float(-0.5f, 10.f),
        rnd::rand_float(-0.95f, 2.f),
        rnd::rand_float(-0.9f, 0.425f),
        rnd::rand_float(-0.9f, 1.5f),
        rnd::rand_float(-1.f, 10.f),
        rnd::rand_float(-1.f, 5.f),
        rnd::rand_float(-0.5f, 3.f),
        rnd::rand_float(-1.f, 4.f),
        rnd::rand_float(-0.5f, 2.f/3.f),
      };
      if (i == 0) // First show the default sound.
       vp.clear();
      auto wd = SFX::generate(SFXType::EXPLOSION, vp, false);
      auto src = src_handler.create_source_from_waveform(wd);
      src->play(PlaybackMode::STATE_WAIT);
    }
  }
  #endif
  
  pressAnyKey();
  
  return 0;
}
