//
//  demo_7.cpp
//  8Beat
//
//  Created by Rasmus Anthin on 2024-03-18.
//

#include "AudioSourceHandler.h"
#include "WaveformGeneration.h"
#include "ChipTuneEngine.h"
#include <Core/Keyboard.h>



int main(int argc, char** argv)
{
  beat::AudioSourceHandler src_handler;
  beat::WaveformGeneration wave_gen;
  
#ifdef _MSC_VER
  std::string wk_dir = "../sounds/";
#else
  std::string wk_dir = "./sounds/";
#endif

  beat::ChipTuneEngine chiptune_engine(src_handler, wave_gen);
  chiptune_engine.load_tune(wk_dir + "chiptune3.ct", true);
  chiptune_engine.set_gain(0.8f);
  chiptune_engine.play_tune(false, false);
  
  if (!keyboard::press_any_key_or_quit())
    return 0;
  
  return 0;
}
