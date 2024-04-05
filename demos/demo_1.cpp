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
  
  // //////////////////////
  
  enum class TestType { TEST_SIMPLE, TEST_WAVE_LAMBDA, TEST_FREQ_LAMBDA, TEST_AMPL_LAMBDA, TEST_ALL_LAMBDA, TEST_ALL_ENUM, TEST_FREQ_SLIDE };
  TestType test = TestType::TEST_FREQ_SLIDE;
  
  auto duration_s = 2.f;
  
  auto freq = 440.f;
  
  audio::Waveform wd;
  
#if 1
  int sample_rate = 44'100;
#else
  int sample_rate = 10'000;
#endif

  audio::WaveformGenerationParams params;
  
  switch (test)
  {
    case TestType::TEST_SIMPLE:
      wd = wave_gen.generate_waveform(audio::WaveformType::SINE, duration_s, freq,
        audio::FrequencyType::CONSTANT, audio::AmplitudeType::CONSTANT, audio::PhaseType::ZERO, params, sample_rate);
      break;
    case TestType::TEST_WAVE_LAMBDA:
#if 1
      wd = wave_gen.generate_waveform(wave_func, duration_s, freq,
                                      audio::FrequencyType::CONSTANT, audio::AmplitudeType::CONSTANT, audio::PhaseType::ZERO,
                                      params, sample_rate);
#else
      wd = wave_gen.generate_waveform([](float phi, float dur) { return std::sin(phi); }, 1e-2f, freq,
        audio::FrequencyType::CONSTANT, audio::AmplitudeType::CONSTANT, audio::PhaseType::ZERO, params, 10'000);
#endif
      break;
    case TestType::TEST_FREQ_LAMBDA:
      wd = wave_gen.generate_waveform(audio::WaveformType::TRIANGLE, duration_s, freq,
                                      freq_func, audio::AmplitudeType::CONSTANT, audio::PhaseType::ZERO, params);
      break;
    case TestType::TEST_AMPL_LAMBDA:
      wd = wave_gen.generate_waveform(audio::WaveformType::SAWTOOTH, duration_s, freq,
                                      audio::FrequencyType::CONSTANT, ampl_func, audio::PhaseType::ZERO, params);
      break;
    case TestType::TEST_ALL_LAMBDA:
      wd = wave_gen.generate_waveform(wave_func, duration_s, freq,
                                      freq_func, ampl_func, audio::PhaseType::ZERO, params);
      break;
    case TestType::TEST_ALL_ENUM:
      wd = wave_gen.generate_waveform(audio::WaveformType::SAWTOOTH, duration_s, freq,
                                      audio::FrequencyType::JET_ENGINE_POWERUP, audio::AmplitudeType::JET_ENGINE_POWERUP, audio::PhaseType::ZERO, params);
      break;
      case TestType::TEST_FREQ_SLIDE:
      params.freq_slide_vel = 0.1f;
      params.freq_slide_acc = -0.1f;
      wd = wave_gen.generate_waveform(audio::WaveformType::SINE, duration_s, freq,
        audio::FrequencyType::CONSTANT, audio::AmplitudeType::CONSTANT, audio::PhaseType::ZERO, params, sample_rate);
      break;
  }
  
#if 0
  auto wd2 = audio.generate_waveform(audio::WaveformType::SINE, duration_s, freq*12);
  auto wd3 = audio.ring_modulation(wd, wd2);
  auto wd4 = audio::WaveformHelper::resample(wd3, 44'100, audio::LowPassFilterType::Butterworth, 2, 1.2f, 1.f);
  auto src = src_handler.create_source_from_waveform(wd4);
  src->play(audio::PlaybackMode::STATE_WAIT);
#else
  auto wd2 = audio::WaveformHelper::resample(wd, 44'100, audio::LowPassFilterType::Butterworth, 2, 1.2f, 1.f);
  auto src = src_handler.create_source_from_waveform(wd2);
  src->play(audio::PlaybackMode::STATE_WAIT);
#endif

  pressAnyKey();
  
  return 0;
}


