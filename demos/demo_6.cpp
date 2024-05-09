//
//  demo_6.cpp
//  8Beat
//
//  Created by Rasmus Anthin on 2024-03-18.
//

#include "../AudioSourceHandler.h"
#include "../WaveformGeneration.h"
#include "../WaveformHelper.h"
#include "../Synthesizer.h"
#include "../../Termin8or/Keyboard.h"
#include <stdio.h>


int main(int argc, char** argv)
{
  enableRawMode();
  
  audio::AudioSourceHandler src_handler;
  audio::WaveformGeneration wave_gen;
  
  
#ifdef _MSC_VER
  std::string wk_dir = "../sounds/";
#else
  std::string wk_dir = "./sounds/";
#endif

#if 0
  // Mysterious fade-in behaviour. #FIXME: Investigate!
  auto wd_sine = wave_gen.generate_waveform(audio::WaveformType::SINE, 3.f, 460.f);
  auto src_sine = src_handler.create_source_from_waveform(wd_sine);
  src_sine->set_volume(0.8f);
  src_sine->play(true);
  audio::WaveformHelper::print_waveform_graph(wd_sine, audio::GraphType::PLOT_THICK0, 150, 20, 0, std::nullopt);
#endif
  std::optional<audio::Waveform> wd_rev_kernel = std::nullopt;
#ifdef USE_SYNTH_REVERB
  wd_rev_kernel = audio::WaveformIO::load(wk_dir + "s1_r1_o.wav", 3);
#endif
  std::vector<audio::Waveform> synth_waves;
  std::vector<audio::AudioSource*> synth_sources;
  for (int i = 0; i < static_cast<int>(audio::InstrumentType::NUM_ITEMS); ++i)
  {
    auto instrument = static_cast<audio::InstrumentType>(i);
    float duration = 2.f;
    if (i >= 6)
      duration = 0.2f;
    float frequency = 440.f;
    if (i == 6)
      frequency = 100.f;
    else if (i == 7)
      frequency = 1200.f;
    auto wd_synth = audio::Synthesizer::synthesize(instrument, wave_gen, duration, frequency,
      44100, false,
      audio::FrequencyType::CONSTANT, audio::AmplitudeType::VIBRATO_0);
    if (wd_rev_kernel.has_value())
      wd_synth = audio::WaveformHelper::reverb_fast(wd_synth, wd_rev_kernel.value());
    //wd_synth = audio::WaveformHelper::fir_flange(wd_synth);
    synth_waves.emplace_back(wd_synth);
    auto* src_synth = src_handler.create_source_from_waveform(wd_synth);
    synth_sources.emplace_back(src_synth);
    src_synth->set_volume(0.8f);
  }
  for (int i = 0; i < static_cast<int>(audio::InstrumentType::NUM_ITEMS); ++i)
  {
    //if (i != 7) continue;
    //if (i != 5) continue;
    auto* src_synth = synth_sources[i];
    const auto& wd_synth = synth_waves[i];
    audio::Synthesizer::print_instrument(static_cast<audio::InstrumentType>(i));
    auto [min_val, max_val] = audio::WaveformHelper::find_min_max(wd_synth);
    std::cout << "(min, max) = (" << min_val << ", " << max_val << ")\n";
    for (int j = 0; j < (i >= 6 ? 3 : 1); ++j)
    {
      src_synth->play(audio::PlaybackMode::STATE_WAIT);
      src_synth->stop();
    }
  }
  for (auto* src_synth : synth_sources)
    src_handler.remove_source(src_synth);
  synth_sources.clear();
  
  pressAnyKey();
  
  return 0;
}
