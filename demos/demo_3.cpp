//
//  demo_3.cpp
//  8Beat
//
//  Created by Rasmus Anthin on 2024-03-17.
//

#include "AudioSourceHandler.h"
#include "WaveformGeneration.h"
#include "WaveformHelper.h"
#include <Core/Keyboard.h>



int main(int argc, char** argv)
{
  beat::AudioSourceHandler src_handler;
  beat::WaveformGeneration wave_gen;
  
  auto wave_func = [](float phi, float /*param*/) -> float
  {
    return std::sin(0.5f*std::sin(phi)*std::sqrt(phi));
  };
  
  auto freq_func = [](float t, float duration, float freq_0) -> float
  {
    float hd = duration * 0.01f;
    if (0.f <= t && t < hd)
      return math::linmap(t, 0.f, hd, 900.f, freq_0);
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
  
  enum class TestType { TEST_SIMPLE, TEST_WAVE_LAMBDA, TEST_FREQ_LAMBDA, TEST_AMPL_LAMBDA, TEST_ALL_LAMBDA, TEST_ALL_ENUM };
  TestType test = TestType::TEST_FREQ_LAMBDA;
  
  auto duration_s = 2.f;
  
  auto freq = 440.f;
  
  beat::Waveform wd;
  
#if 1
  int sample_rate = 44'100;
#else
  int sample_rate = 10'000;
#endif

  beat::WaveformGenerationParams params;

  switch (test)
  {
    case TestType::TEST_SIMPLE:
      wd = wave_gen.generate_waveform(beat::WaveformType::SINE, duration_s, freq,
                                      params, sample_rate);
      break;
    case TestType::TEST_WAVE_LAMBDA:
      wd = wave_gen.generate_waveform(wave_func, duration_s, freq,
                                      params, sample_rate);
      break;
    case TestType::TEST_FREQ_LAMBDA:
      wd = wave_gen.generate_waveform(beat::WaveformType::TRIANGLE, duration_s, freq,
                                      params, sample_rate, false,
                                      freq_func);
      break;
    case TestType::TEST_AMPL_LAMBDA:
      wd = wave_gen.generate_waveform(beat::WaveformType::SAWTOOTH, duration_s, freq,
                                      params, sample_rate, false,
                                      beat::FrequencyType::CONSTANT, ampl_func);
      break;
    case TestType::TEST_ALL_LAMBDA:
      wd = wave_gen.generate_waveform(wave_func, duration_s, freq,
                                      params, sample_rate, false,
                                      freq_func, ampl_func);
      break;
    case TestType::TEST_ALL_ENUM:
      wd = wave_gen.generate_waveform(beat::WaveformType::SAWTOOTH, duration_s, freq,
                                      params, sample_rate, false,
                                      beat::FrequencyType::JET_ENGINE_POWERUP, beat::AmplitudeType::JET_ENGINE_POWERUP);
      break;
  }
  auto wd2 = beat::WaveformHelper::resample(wd, 44'100, beat::FilterType::Butterworth, 2, 1.2f, 1.f);
  auto src = src_handler.create_source_from_waveform(wd2);
  src->play(beat::PlaybackMode::STATE_WAIT);
  
// Plot Audio

  std::cout << "Please make sure your terminal window is at least 150 chars wide.\n";
  if (!keyboard::press_any_key_or_quit())
    return 0;

  int width = 150;
  int height = 20;
  
#define USE_TIME_INTERVAL
#ifdef USE_TIME_INTERVAL
  auto T = beat::WaveformHelper::calc_time_from_num_cycles(wd2, 1);
  float start = 0.f; //100*T;
  //float dt = 1.f/wd.sample_rate;
  //float end = 100*dt;
  float end = start + 10*T;
#else
  int start = 0;
  std::optional<int> end = std::nullopt;//400;
#endif

  auto print_waveform_graph = [](const beat::Waveform& wave, beat::GraphType graph_type,
                                 int width, int height,
                                 auto start, auto end)
  {
#ifdef USE_TIME_INTERVAL
    beat::WaveformHelper::print_waveform_graph_t(wave, graph_type, width, height, start, end);
#else
    beat::WaveformHelper::print_waveform_graph_idx(wave, graph_type, width, height, start, end);
#endif
  };

  std::cout << "plot thin:\n";
  print_waveform_graph(wd2, beat::GraphType::PLOT_THIN, width, height, start, end);
  std::cout << "plot thick 0:\n";
  print_waveform_graph(wd2, beat::GraphType::PLOT_THICK0, width, height, start, end);
  std::cout << "plot thick 1:\n";
  print_waveform_graph(wd2, beat::GraphType::PLOT_THICK1, width, height, start, end);
  std::cout << "plot thick 2:\n";
  print_waveform_graph(wd2, beat::GraphType::PLOT_THICK2, width, height, start, end);
  std::cout << "plot thick 3:\n";
  print_waveform_graph(wd2, beat::GraphType::PLOT_THICK3, width, height, start, end);
  std::cout << "filled bottom-up:\n";
  print_waveform_graph(wd2, beat::GraphType::FILLED_BOTTOM_UP, width, height, start, end);
  std::cout << "filled from t-axis:\n";
  print_waveform_graph(wd2, beat::GraphType::FILLED_FROM_T_AXIS, width, height, start, end);
  
  if (!keyboard::press_any_key_or_quit())
    return 0;
    
  return 0;
}
