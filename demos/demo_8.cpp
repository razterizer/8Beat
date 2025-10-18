//
//  demo_8.cpp
//  8Beat
//
//  Created by Rasmus Anthin on 2024-03-18.
//

#include "AudioSourceHandler.h"
#include "WaveformGeneration.h"
#include <Termin8or/input/Keyboard.h>


class Test : public beat::AudioStreamListener
{
  virtual bool has_mono() const override { return true; }
  virtual bool has_stereo() const override { return true; }
  virtual float on_get_sample_mono(float t) const override
  {
    return std::sin(math::c_2pi*350*t);
  }
  virtual std::pair<float, float> on_get_sample_stereo(float t) const override
  {
    //std::cout << t << std::endl;
    auto f = 320.f;
    return { std::sin(math::c_2pi*f*t), /*std::fmod(f*t, 1.f)*/ /*0.f*/ std::sin(math::c_2pi*f*t) };
  }
};


int main(int argc, char** argv)
{
  t8::StreamKeyboard keyboard;

  beat::AudioSourceHandler src_handler;
  beat::WaveformGeneration wave_gen;

  Test test_sample_gen;
  auto* stream_src_0 = src_handler.create_stream_source(&test_sample_gen, 44100);
  
  auto* src_1 = src_handler.create_source();
  auto* src_2 = src_handler.create_source();
  auto wave_sine = wave_gen.generate_waveform(beat::WaveformType::SINE, 3.f, 440.f);
  auto wave_square = wave_gen.generate_waveform(beat::WaveformType::SQUARE, 3.f, 360.f);
  auto wave_triangle = wave_gen.generate_waveform(beat::WaveformType::TRIANGLE, 3.f, 1298.f);
  
  int num_tot_samples = 100'000; // ~2s
  
  std::cout << "Listener-based AudioStreamSource : mono" << std::endl;
  stream_src_0->set_volume(0.2f);
  stream_src_0->update_buffer(num_tot_samples, 1);
  stream_src_0->play(beat::PlaybackMode::STATE_WAIT);
  
  std::cout << "Listener-based AudioStreamSource : stereo" << std::endl;
  stream_src_0->set_volume(0.2f);
  stream_src_0->update_buffer(num_tot_samples, 2);
  stream_src_0->play(beat::PlaybackMode::NONE);
  float t = 0.f;
  int num_iters = 10; //1'000;
  for (int i = 0; i < num_iters; ++i)
  {
    t = static_cast<float>(i)/num_iters;
    stream_src_0->set_panning(t);
    Delay::sleep(stream_src_0->get_duration_s()/num_iters*1e6);
  }

  std::cout << "Buffer-based AudioStreamSource" << std::endl;
  std::cout << "  Setting volumes" << std::endl;
  src_1->set_volume(0.2f);
  src_2->set_volume(0.2f);
  
  std::cout << "  Setting buffers: source 1 <- wave_sine, source 2 <- wave_square" << std::endl;
  src_1->update_buffer(wave_sine);
  src_2->update_buffer(wave_square);
  std::cout << "  Playing buffers concurrently with common sleep" << std::endl;
  src_1->play(beat::PlaybackMode::NONE);
  src_2->play(beat::PlaybackMode::NONE);
  Delay::sleep(3*1e6);
  
  std::cout << "  Stopping source 1" << std::endl;
  src_1->stop();
  std::cout << "  Updating buffer 2: source 2 <- wave_triangle" << std::endl;
  src_2->update_buffer(wave_triangle);
  std::cout << "  Playing buffers concurrently with common sleep" << std::endl;
  src_1->play(beat::PlaybackMode::NONE);
  src_2->play(beat::PlaybackMode::NONE);
  Delay::sleep(3*1e6);
  
  src_handler.remove_source(stream_src_0);
  src_handler.remove_source(src_1);
  src_handler.remove_source(src_2);
  
  keyboard.pressAnyKey();
  
  return 0;
}
