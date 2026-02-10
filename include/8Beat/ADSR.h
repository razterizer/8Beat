//
//  ADSR.h
//  8Beat
//
//  Created by Rasmus Anthin on 2024-02-22.
//

#pragma once

#define ADSR_ENTRY(name) { #name, name },

#include <map>
#include <optional>
#include <string>


namespace beat
{

  enum class ADSRMode { LIN, EXP, LOG };
  
  struct Attack
  {
    Attack() = default;
    Attack(ADSRMode m, float time_ms,
           std::optional<float> lvl_0 = std::nullopt, std::optional<float> lvl_1 = std::nullopt)
      : mode(m)
      , attack_time_ms(time_ms)
      , level_0(lvl_0)
      , level_1(lvl_1)
    {}
    ADSRMode mode = ADSRMode::LIN;
    float attack_time_ms = 0.f;
    std::optional<float> level_0;
    std::optional<float> level_1;
  };
  struct Decay
  {
    Decay() = default;
    Decay(ADSRMode m, float time_ms,
          std::optional<float> lvl_0 = std::nullopt, std::optional<float> lvl_1 = std::nullopt)
      : mode(m)
      , decay_time_ms(time_ms)
      , level_0(lvl_0)
      , level_1(lvl_1)
    {}
    ADSRMode mode = ADSRMode::LIN;
    float decay_time_ms = 0.f;
    std::optional<float> level_0;
    std::optional<float> level_1;
  };
  struct Sustain
  {
    Sustain() = default;
    Sustain(float level, std::optional<float> max_time_ms = std::nullopt)
      : sustain_level(level)
      , max_sustain_time_ms(max_time_ms)
    {}
    float sustain_level = 0.5f;
    std::optional<float> max_sustain_time_ms;
  };
  struct Release
  {
    Release() = default;
    Release(ADSRMode m, float time_ms,
            std::optional<float> lvl_0 = std::nullopt, std::optional<float> lvl_1 = std::nullopt)
      : mode(m)
      , release_time_ms(time_ms)
      , level_0(lvl_0)
      , level_1(lvl_1)
    {}
    ADSRMode mode = ADSRMode::LIN;
    float release_time_ms = 0.f;
    std::optional<float> level_0;
    std::optional<float> level_1;
  };
  class ADSR
  {
    Attack attack;
    Decay decay;
    Sustain sustain;
    Release release;
  
    float level_a0 = 0.f;
    float level_a1 = 1.f;
    float level_d0 = 1.f;
    float level_d1 = 0.8f;
    float level_s = 0.8f;
    float level_r0 = 0.8f;
    float level_r1 = 0.f;
    
  public:
    ADSR() = default;
    ADSR(const Attack& a, const Decay& d, const Sustain& s, const Release& r)
      : attack(a), decay(d), sustain(s), release(r)
      , level_a0(a.level_0.value_or(0.f))
      , level_a1(a.level_1.value_or(d.level_0.value_or(1.f)))
      , level_d0(d.level_0.value_or(a.level_1.value_or(1.f)))
      , level_d1(d.level_1.value_or(s.sustain_level))
      , level_s(s.sustain_level)
      , level_r0(r.level_0.value_or(s.sustain_level))
      , level_r1(r.level_1.value_or(0.f))
    {
      //adjust_levels();
    }
    
    void adjust_levels()
    {
      level_a0 = attack.level_0.value_or(0.f);
      level_a1 = attack.level_1.value_or(decay.level_0.value_or(1.f));
      level_d0 = decay.level_0.value_or(attack.level_1.value_or(1.f));
      level_d1 = decay.level_1.value_or(sustain.sustain_level);
      level_s = sustain.sustain_level;
      level_r0 = release.level_0.value_or(sustain.sustain_level);
      level_r1 = release.level_1.value_or(0.f);
    }
    
    float get_level_A0() const { return level_a0; }
    float get_level_A1() const { return level_a1; }
    float get_level_D0() const { return level_d0; }
    float get_level_D1() const { return level_d1; }
    float get_level_S() const { return level_s; }
    float get_level_R0() const { return level_r0; }
    float get_level_R1() const { return level_r1; }
    
    float get_time_A_ms() const { return attack.attack_time_ms; }
    float get_time_D_ms() const { return decay.decay_time_ms; }
    std::optional<float> get_max_time_S_ms() const { return sustain.max_sustain_time_ms; }
    float get_time_R_ms() const { return release.release_time_ms; }
    
    ADSRMode get_shape_A() const { return attack.mode; }
    ADSRMode get_shape_D() const { return decay.mode; }
    ADSRMode get_shape_R() const { return release.mode; }
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
    static ADSR GUITAR
    {
      { ADSRMode::EXP, 20 },   // Attack: Exponential rise, adjust time as needed
      { ADSRMode::LIN, 150 },  // Decay: Linear decay, adjust time as needed
      { 0.5f },                // Sustain: Sustain level, adjust as needed
      { ADSRMode::LOG, 100 }   // Release: Logarithmic release, adjust time as needed
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
    
    class ADSR_Presets
    {
      std::map<std::string, ADSR> m_envelopes
      {
        ADSR_ENTRY(PIANO_0)
        ADSR_ENTRY(PIANO_1)
        ADSR_ENTRY(PIANO_2)
        ADSR_ENTRY(PIANO_3)
        ADSR_ENTRY(VIOLIN_0)
        ADSR_ENTRY(VIOLIN_1)
        ADSR_ENTRY(VIOLIN_2)
        ADSR_ENTRY(VIOLIN_3)
        ADSR_ENTRY(ORGAN_0)
        ADSR_ENTRY(ORGAN_1)
        ADSR_ENTRY(ORGAN_2)
        ADSR_ENTRY(ORGAN_3)
        ADSR_ENTRY(ORGAN_4)
        ADSR_ENTRY(TRUMPET_0)
        ADSR_ENTRY(TRUMPET_1)
        ADSR_ENTRY(TRUMPET_2)
        ADSR_ENTRY(TRUMPET_3)
        ADSR_ENTRY(TRUMPET_4)
        ADSR_ENTRY(FLUTE_0)
        ADSR_ENTRY(FLUTE_1)
        ADSR_ENTRY(FLUTE_2)
        ADSR_ENTRY(FLUTE_3)
        ADSR_ENTRY(GUITAR)
        ADSR_ENTRY(KICKDRUM)
        ADSR_ENTRY(SNAREDRUM)
        ADSR_ENTRY(HIHAT)
      };
      
    public:
      ADSR get(const std::string& adsr_name)
      {
        auto it = m_envelopes.find(adsr_name);
        if (it != m_envelopes.end())
          return it->second;
        else
          return {};
      }
    };
    
    static ADSR_Presets getter;
    
  }
  
}
