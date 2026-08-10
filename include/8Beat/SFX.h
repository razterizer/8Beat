//
//  SFX.h
//  8Beat
//
//  Created by Rasmus Anthin on 2024-05-18.
//

#pragma once

#include "WaveformGeneration.h"
#include "WaveformHelper.h"


namespace beat
{
  enum class SFXType
  {
    COIN,
    LASER,
    EXPLOSION,
    NUM_ITEMS
  };

  class SFX
  {
  public:
    static Waveform generate(SFXType type,
                             const std::vector<float>& variation_params = {},
                             bool verbose = false)
    {
      if (verbose)
      {
        std::cout << "Variation Params:" << std::endl;
        for (auto v : variation_params)
          std::cout << "  " << v << std::endl;
      }
      auto vp = [&variation_params](int idx)
      {
        return 1.f + stlutils::try_get(variation_params, idx, 0.f);
      };
      
      WaveformGeneration wave_gen;
      WaveformGenerationParams params;
      Waveform wd;
      
      switch (type)
      {
        case SFXType::COIN:
          if (verbose && variation_params.size() > 10)
            std::cout << "You only need 10 parameters for the COIN SFX." << std::endl;
          params.arpeggio.emplace_back(0.1f*vp(0), 1.274f*vp(1));
          params.duty_cycle = 0.5f*vp(2);
          
          wd = wave_gen.generate_waveform(WaveformType::TRIANGLE, 0.5f*vp(3), 1369.f*vp(4),
                                          params, 44100, false);
          wd = WaveformHelper::envelope_adsr(wd,
                                             Attack { ADSRMode::LIN, 50*vp(5), 0.f, 0.2f*vp(8) },
                                             Decay { ADSRMode::LIN, 100*vp(6), 1.f},
                                             Sustain { 0.4f*vp(9) },
                                             Release { ADSRMode::EXP, 150*vp(7) });
          return wd;
        case SFXType::LASER:
          if (verbose && variation_params.size() > 12)
            std::cout << "You only need 12 parameters for the LASER SFX." << std::endl;
          params.freq_slide_vel = math::lerp(vp(0)-1.f, -20.49f, 30.f); // def:-20.49
          params.duty_cycle = 0.4762f*vp(1);
          params.duty_cycle_sweep = math::lerp(vp(2)-1.f, -1.821f, 2.f)/100; // def:-1.821/100
          params.max_frequency_limit = 44100*0.5f;
          
          wd = wave_gen.generate_waveform(WaveformType::SQUARE, 0.2f*vp(3), 968.6f*vp(4),
                                          params, 44100, false);
          wd = WaveformHelper::envelope_adsr(wd,
                                             Attack { ADSRMode::LIN, 0, 0.f, 0.2f*vp(7) },
                                             Decay { ADSRMode::LIN, 20*vp(5), 1.f},
                                             Sustain { 0.4f*vp(8) },
                                             Release { ADSRMode::EXP, 13*vp(6) });
          {
            float delay_time = 3e-3f*vp(9);
            float rate = 0.9f*vp(10);
            float feedback = 0.9f*vp(11);
            wd = WaveformHelper::flanger(wd, delay_time, rate, feedback);
          }
          return wd;
        case SFXType::EXPLOSION:
          if (verbose && variation_params.size() > 17)
            std::cout << "You only need 17 parameters for the EXPLOSION SFX." << std::endl;
          params.freq_slide_vel = math::lerp(vp(0), -2.f, 2.f); // def:2
          params.freq_slide_acc = math::lerp(vp(1), -5.f, 5.f); // def:5
          params.vibrato_depth = 0.1421f*vp(2); // def:0.1421
          params.vibrato_freq = 10.16f*vp(3); // def:10.16
          params.noise_filter_rel_bw = 0.1f*vp(4); // def:0.1f
          params.noise_filter_order = static_cast<int>(std::round(vp(5))); // def:1
          params.noise_filter_slot_dur_s = 1e-3f*vp(6); // def:1e-3
          
          wd = wave_gen.generate_waveform(WaveformType::NOISE, 0.8f*vp(7), 15.f*vp(8),
                                          params, 44100, false);
          
          {
            float delay_time = 1e-3f*vp(9);
            float rate = 0.7f*vp(10);
            float feedback = 0.4f*vp(11);
            wd = WaveformHelper::flanger(wd, delay_time, rate, feedback);
          }
          
          wd = WaveformHelper::envelope_adsr(wd,
                                             Attack { ADSRMode::LIN, 10*vp(12), 0.f, 0.2f*vp(15) },
                                             Decay { ADSRMode::LIN, 50*vp(13), 1.f },
                                             Sustain { 0.6f*vp(16) },
                                             Release { ADSRMode::LIN, 340*vp(14) });
          
          WaveformHelper::normalize(wd);
          return wd;
        case SFXType::NUM_ITEMS:
          return wd;
      }
      return wd;
    }
  };
  
}
