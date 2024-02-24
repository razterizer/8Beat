//
//  ADSR.h
//  8-Bit Audio Emulator Lib
//
//  Created by Rasmus Anthin on 2024-02-22.
//

#pragma once

namespace audio
{

  enum class ADSRMode { LIN, EXP, LOG };
  
  struct Attack
  {
    Attack() = default;
    Attack(ADSRMode m, float time_ms)
      : mode(m)
      , attack_time_ms(time_ms)
    {}
    ADSRMode mode = ADSRMode::LIN;
    float attack_time_ms = 0.f;
  };
  struct Decay
  {
    Decay() = default;
    Decay(ADSRMode m, float time_ms)
      : mode(m)
      , decay_time_ms(time_ms)
    {}
    ADSRMode mode = ADSRMode::LIN;
    float decay_time_ms = 0.f;
  };
  struct Sustain
  {
    Sustain() = default;
    Sustain(float level)
      : sustain_level(level)
    {}
    float sustain_level = 0.f;
  };
  struct Release
  {
    Release() = default;
    Release(ADSRMode m, float time_ms)
      : mode(m)
      , release_time_ms(time_ms)
    {}
    ADSRMode mode = ADSRMode::LIN;
    float release_time_ms = 0.f;
  };
  struct ADSR
  {
    ADSR() = default;
    ADSR(const Attack& a, const Decay& d, const Sustain& s, const Release& r)
      : attack(a), decay(d), sustain(s), release(r)
    {}
  
    Attack attack;
    Decay decay;
    Sustain sustain;
    Release release;
  };
  
  namespace adsr_presets
  {
    static ADSR PIANO_0
    {
      { ADSRMode::LIN, 10 },
      { ADSRMode::LIN, 300 },
      { 0.3f },
      { ADSRMode::EXP, 500 },
    };
    static ADSR PIANO_1
    {
      { ADSRMode::EXP, 20 },
      { ADSRMode::LIN, 500 },
      { 0.5f },
      { ADSRMode::LIN, 400 },
    };
    static ADSR PIANO_2
    {
      { ADSRMode::LIN, 15 },
      { ADSRMode::EXP, 400 },
      { 0.4f },
      { ADSRMode::EXP, 450 },
    };
    static ADSR PIANO_3
    {
      { ADSRMode::EXP, 25 },
      { ADSRMode::EXP, 600 },
      { 0.6f },
      { ADSRMode::EXP, 350 },
    };
    static ADSR VIOLIN_0
    {
      { ADSRMode::LIN, 10 },
      { ADSRMode::EXP, 200 },
      { 0.4f },
      { ADSRMode::EXP, 300 },
    };
    static ADSR VIOLIN_1
    {
      { ADSRMode::EXP, 15 },
      { ADSRMode::LIN, 300 },
      { 0.5f },
      { ADSRMode::LIN, 250 },
    };
    static ADSR VIOLIN_2
    {
      { ADSRMode::LIN, 12 },
      { ADSRMode::EXP, 250 },
      { 0.45f },
      { ADSRMode::LIN, 280 },
    };
    static ADSR VIOLIN_3
    {
      { ADSRMode::EXP, 8 },
      { ADSRMode::LIN, 180 },
      { 0.35f },
      { ADSRMode::EXP, 320 },
    };
    static ADSR ORGAN_0
    {
      { ADSRMode::EXP, 5 },
      { ADSRMode::LIN, 800 },
      { 0.7f },
      { ADSRMode::EXP, 300 },
    };
    static ADSR ORGAN_1
    {
      { ADSRMode::LIN, 5 },
      { ADSRMode::EXP, 400 },
      { 0.6f },
      { ADSRMode::EXP, 200 },
    };
    static ADSR ORGAN_2
    {
      { ADSRMode::EXP, 8 },
      { ADSRMode::LIN, 600 },
      { 0.7f },
      { ADSRMode::LIN, 180 },
    };
    static ADSR ORGAN_3
    {
      { ADSRMode::LIN, 6 },
      { ADSRMode::EXP, 500 },
      { 0.65f },
      { ADSRMode::LIN, 190 },
    };
    static ADSR ORGAN_4
    {
      { ADSRMode::EXP, 10 },
      { ADSRMode::LIN, 700 },
      { 0.75f },
      { ADSRMode::EXP, 160 },
    };
    static ADSR TRUMPET_0
    {
      { ADSRMode::LOG, 2 },
      { ADSRMode::EXP, 200 },
      { 0.8f },
      { ADSRMode::LOG, 100 },
    };
    static ADSR TRUMPET_1
    {
      { ADSRMode::LOG, 3 },
      { ADSRMode::EXP, 100 },
      { 0.8f },
      { ADSRMode::LOG, 50 },
    };
    static ADSR TRUMPET_2
    {
      { ADSRMode::EXP, 4 },
      { ADSRMode::LIN, 120 },
      { 0.7f },
      { ADSRMode::LOG, 60 },
    };
    static ADSR TRUMPET_3
    {
      { ADSRMode::LOG, 2 },
      { ADSRMode::EXP, 80 },
      { 0.85f },
      { ADSRMode::EXP, 40 },
    };
    static ADSR TRUMPET_4
    {
      { ADSRMode::LOG, 5 },
      { ADSRMode::EXP, 150 },
      { 0.75f },
      { ADSRMode::LIN, 70 },
    };
    static ADSR FLUTE_0
    {
      { ADSRMode::EXP, 6 },
      { ADSRMode::LIN, 250 },
      { 0.6f },
      { ADSRMode::EXP, 200 },
    };
    static ADSR FLUTE_1
    {
      { ADSRMode::EXP, 8 },
      { ADSRMode::EXP, 300 },
      { 0.7f },
      { ADSRMode::LIN, 180 },
    };
    static ADSR FLUTE_2
    {
      { ADSRMode::LIN, 5 },
      { ADSRMode::EXP, 200 },
      { 0.65f },
      { ADSRMode::EXP, 210 },
    };
    static ADSR FLUTE_3
    {
      { ADSRMode::EXP, 10 },
      { ADSRMode::LIN, 350 },
      { 0.8f },
      { ADSRMode::LIN, 160 },
    };
    static ADSR KICKDRUM
    {
      { ADSRMode::LOG, 5 },
      { ADSRMode::LIN, 50 },
      { 0.6f },
      { ADSRMode::LOG, 20 },
    };
    static ADSR SNAREDRUM
    {
      { ADSRMode::LOG, 3 },
      { ADSRMode::LIN, 80 },
      { 0.4f },
      { ADSRMode::LOG, 30 },
    };
    static ADSR HIHAT
    {
      { ADSRMode::LOG, 1 },
      { ADSRMode::EXP, 20 },
      { 0.2f },
      { ADSRMode::LOG, 10 },
    };
  }
}
