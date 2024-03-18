//
//  demo_7.cpp
//  8Beat
//
//  Created by Rasmus Anthin on 2024-03-18.
//

#include "../AudioSourceHandler.h"
#include "../WaveformGeneration.h"
#include "../ChipTuneEngine.h"



int main(int argc, char** argv)
{
  audio::AudioSourceHandler src_handler;
  audio::WaveformGeneration wave_gen;
  
#ifdef _MSC_VER
  std::string wk_dir = "../sounds/";
#else
  std::string wk_dir = "./sounds/";
#endif

  audio::ChipTuneEngine chiptune_engine(src_handler, wave_gen);
  chiptune_engine.load_tune(wk_dir + "chiptune3.ct");
  chiptune_engine.play_tune();
  
  return 0;
}