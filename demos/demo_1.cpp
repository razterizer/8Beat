//
//  demo_1.cpp
//  8Beat
//
//  Created by Rasmus Anthin on 2024-03-16.
//

#include "../AudioSourceHandler.h"
#include "../WaveformGeneration.h"
#include "../WaveformHelper.h"
#include "../../Termin8or/Keyboard.h"


int main(int argc, char** argv)
{
  enableRawMode();

  audio::AudioSourceHandler src_handler;
  audio::WaveformGeneration wave_gen;
  
  auto wave_func = [](float phi, float /*param*/) -> float
  {
    return std::sin(0.5f*std::sin(phi)*std::sqrt(phi));
  };
  
  auto freq_func = [](float t, float duration, float freq_0) -> float
  {
    float hd = duration * 0.5f;
    if (0.f <= t && t < hd)
      return math::linmap(t, 0.f, hd, 800.f, freq_0);
    else if(hd <= t && t <= duration)
      return math::linmap(t, hd, duration, freq_0, 800.f);
    else
      return freq_0;
  };
  
  auto ampl_func = [](float t, float duration)
  {
    float hd = duration * 0.5f;
    if (0.f <= t && t < hd)
      return math::linmap(t, 0.f, hd, 0.1f, 1.f);
    else if(hd <= t && t <= duration)
      return math::linmap(t, hd, duration, 1.f, 0.f);
    else
      return 1.f;
  };
  
  auto phase_func = [](float t, float duration)
  {
    return math::linmap(t, 0.f, duration, 1e-7f, 1e-4f);
  };
  
  // //////////////////////
  
  enum class TestType { SIMPLE, WAVE_LAMBDA, FREQ_LAMBDA, AMPL_LAMBDA, PHASE_LAMBDA, ALL_LAMBDA, ALL_ENUM, FREQ_SLIDE, VIBRATO, ARPEGGIO, DUTY_CYCLE_SQUARE, DUTY_CYCLE_TRIANGLE, DUTY_CYCLE_SAWTOOTH, NUM_ITEMS };
  std::vector<std::string> test_labels { "SIMPLE", "WAVE_LAMBDA", "FREQ_LAMBDA", "AMPL_LAMBDA", "PHASE_LAMBDA", "ALL_LAMBDA", "ALL_ENUM", "FREQ_SLIDE", "VIBRATO", "ARPEGGIO", "DUTY_CYCLE_SQUARE", "DUTY_CYCLE_TRIANGLE", "DUTY_CYCLE_SAWTOOTH", "" };
  
  auto duration_s = 2.f;
  
  auto freq = 440.f;
  
  audio::Waveform wd;
  
#if 1
  int sample_rate = 44'100;
#else
  int sample_rate = 10'000;
#endif
  
  for (int test_idx = 0; test_idx < static_cast<int>(TestType::NUM_ITEMS); ++test_idx)
  {
    auto test = static_cast<TestType>(test_idx);
    
    auto label = test_labels[test_idx];
    std::cout << label << ":" << std::endl;
    
    pressAnyKey();
    
    audio::WaveformGenerationParams params;
    
    switch (test)
    {
      case TestType::SIMPLE:
        wd = wave_gen.generate_waveform(audio::WaveformType::SINE, duration_s, freq,
                                        params, sample_rate, false,
                                        audio::FrequencyType::CONSTANT, audio::AmplitudeType::CONSTANT, audio::PhaseType::ZERO);
        break;
      case TestType::WAVE_LAMBDA:
#if 1
        wd = wave_gen.generate_waveform(wave_func, duration_s, freq,
                                        params, sample_rate, false,
                                        audio::FrequencyType::CONSTANT, audio::AmplitudeType::CONSTANT, audio::PhaseType::ZERO);
#else
        wd = wave_gen.generate_waveform([](float phi, float dur) { return std::sin(phi); }, 1e-2f, freq,
                                        params, 10'000, false,
                                        audio::FrequencyType::CONSTANT, audio::AmplitudeType::CONSTANT, audio::PhaseType::ZERO);
#endif
        break;
      case TestType::FREQ_LAMBDA:
        wd = wave_gen.generate_waveform(audio::WaveformType::TRIANGLE, duration_s, freq,
                                        params, sample_rate, false,
                                        freq_func, audio::AmplitudeType::CONSTANT, audio::PhaseType::ZERO);
        break;
      case TestType::AMPL_LAMBDA:
        wd = wave_gen.generate_waveform(audio::WaveformType::SAWTOOTH, duration_s, freq,
                                        params, sample_rate, false,
                                        audio::FrequencyType::CONSTANT, ampl_func, audio::PhaseType::ZERO);
        break;
      case TestType::PHASE_LAMBDA:
        wd = wave_gen.generate_waveform(audio::WaveformType::SQUARE, duration_s, freq,
                                        params, sample_rate, false,
                                        audio::FrequencyType::CONSTANT, audio::AmplitudeType::CONSTANT,
                                        phase_func);
        break;
      case TestType::ALL_LAMBDA:
        wd = wave_gen.generate_waveform(wave_func, duration_s, freq,
                                        params, sample_rate, false,
                                        freq_func, ampl_func, phase_func);
        break;
      case TestType::ALL_ENUM:
        wd = wave_gen.generate_waveform(audio::WaveformType::SAWTOOTH, duration_s, freq,
                                        params, sample_rate, false,
                                        audio::FrequencyType::JET_ENGINE_POWERUP, audio::AmplitudeType::JET_ENGINE_POWERUP, audio::PhaseType::ZERO);
        break;
      case TestType::FREQ_SLIDE:
        params.freq_slide_vel = 0.1f;
        params.freq_slide_acc = -0.1f;
        wd = wave_gen.generate_waveform(audio::WaveformType::SINE, duration_s, freq,
                                        params, sample_rate, false,
                                        audio::FrequencyType::CONSTANT, audio::AmplitudeType::CONSTANT, audio::PhaseType::ZERO);
        break;
      case TestType::VIBRATO:
        params.vibrato_depth = 0.2f;
        params.vibrato_freq = 2.2f;
        params.vibrato_freq_vel = 1e-4f;
        params.vibrato_freq_acc = -2e-4f;
        params.vibrato_freq_acc_max_vel_limit = 2.f;
        wd = wave_gen.generate_waveform(audio::WaveformType::SINE, duration_s, freq,
                                        params, sample_rate, false,
                                        audio::FrequencyType::CONSTANT, audio::AmplitudeType::CONSTANT, audio::PhaseType::ZERO);
        break;
      case TestType::ARPEGGIO:
        params.arpeggio.emplace_back(0.2f, 1.5f);
        params.arpeggio.emplace_back(0.8f, 1.9f);
        wd = wave_gen.generate_waveform(audio::WaveformType::SINE, duration_s, freq,
                                        params, sample_rate, false,
                                        audio::FrequencyType::CONSTANT, audio::AmplitudeType::CONSTANT, audio::PhaseType::ZERO);
        break;
      case TestType::DUTY_CYCLE_SQUARE:
        params.duty_cycle = 0.2f;
        params.duty_cycle_sweep = 0.05f;
        wd = wave_gen.generate_waveform(audio::WaveformType::SQUARE, duration_s, freq,
                                        params, sample_rate, false,
                                        audio::FrequencyType::CONSTANT, audio::AmplitudeType::CONSTANT, audio::PhaseType::ZERO);
        break;
      case TestType::DUTY_CYCLE_TRIANGLE:
        params.duty_cycle = 0.5f;
        params.duty_cycle_sweep = -0.25f;
        wd = wave_gen.generate_waveform(audio::WaveformType::TRIANGLE, duration_s, freq,
                                        params, sample_rate, false,
                                        audio::FrequencyType::CONSTANT, audio::AmplitudeType::CONSTANT, audio::PhaseType::ZERO);
        break;
      case TestType::DUTY_CYCLE_SAWTOOTH:
        params.duty_cycle = 1.f;
        params.duty_cycle_sweep = -0.5f;
        wd = wave_gen.generate_waveform(audio::WaveformType::SAWTOOTH, duration_s, freq,
                                        params, sample_rate, false,
                                        audio::FrequencyType::CONSTANT, audio::AmplitudeType::CONSTANT, audio::PhaseType::ZERO);
        break;
      default:
        break;
    }
    
#if 0
    auto wd2 = audio.generate_waveform(audio::WaveformType::SINE, duration_s, freq*12);
    auto wd3 = audio.ring_modulation(wd, wd2);
    auto wd4 = audio::WaveformHelper::resample(wd3, 44'100, audio::FilterType::Butterworth, 2, 1.2f, 1.f);
    auto src = src_handler.create_source_from_waveform(wd4);
    src->play(audio::PlaybackMode::STATE_WAIT);
#else
    auto wd2 = audio::WaveformHelper::resample(wd, 44'100, audio::FilterType::Butterworth, 2, 1.2f, 1.f);
    auto src = src_handler.create_source_from_waveform(wd2);
    src->play(audio::PlaybackMode::STATE_WAIT);
#endif
  }
  
  return 0;
}


