//
//  demo_8.cpp
//  8Beat
//
//  Created by Rasmus Anthin on 2024-03-18.
//

#include "../AudioSourceHandler.h"
#include "../WaveformGeneration.h"
#include <Termin8or/Keyboard.h>


class Test : public audio::AudioStreamListener
{
  virtual float on_get_sample(float t) const override
  {
    return std::sin(math::c_2pi*440*t);
  }
};


int main(int argc, char** argv)
{
  enableRawMode();

  audio::AudioSourceHandler src_handler;
  audio::WaveformGeneration wave_gen;

//#define USE_CALLBACK
#ifdef USE_CALLBACK
  Test test_sample_gen;
  auto* stream_src_1 = src_handler.create_stream_source(&test_sample_gen, 44100);
#else
  auto* stream_src_1 = src_handler.create_stream_source();
  auto* stream_src_2 = src_handler.create_stream_source();
  auto wave_sine = wave_gen.generate_waveform(audio::WaveformType::SINE, 3.f, 440.f);
  auto wave_square = wave_gen.generate_waveform(audio::WaveformType::SQUARE, 3.f, 360.f);
  auto wave_triangle = wave_gen.generate_waveform(audio::WaveformType::TRIANGLE, 3.f, 1298.f);
#endif
  
#ifdef USE_CALLBACK
  stream_src_1->set_volume(0.2f);
  stream_src_1->update_buffer(20000);
  stream_src_1->play(audio::PlaybackMode::STATE_WAIT);
  src_handler.remove_source(stream_src_1);
#else
  stream_src_1->set_volume(0.2f);
  stream_src_2->set_volume(0.2f);
  
  stream_src_1->update_buffer(wave_sine);
  stream_src_2->update_buffer(wave_square);
  stream_src_1->play(audio::PlaybackMode::NONE);
  stream_src_2->play(audio::PlaybackMode::NONE);
  Delay::sleep(3*1e6);
  
  stream_src_1->stop();
  stream_src_2->update_buffer(wave_triangle);
  stream_src_1->play(audio::PlaybackMode::NONE);
  stream_src_2->play(audio::PlaybackMode::NONE);
  Delay::sleep(3*1e6);
  
  src_handler.remove_source(stream_src_1);
  src_handler.remove_source(stream_src_2);
#endif
  
  pressAnyKey();
  
  return 0;
}
