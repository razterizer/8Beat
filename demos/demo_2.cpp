//
//  demo_2.cpp
//  8Beat
//
//  Created by Rasmus Anthin on 2024-03-16.
//

#include "AudioSourceHandler.h"
#include "WaveformGeneration.h"
#include "WaveformHelper.h"
#include "WaveformIO.h"
#include <Termin8or/input/Keyboard.h>
#include <filesystem>


int main(int argc, char** argv)
{
  t8::StreamKeyboard keyboard;

  beat::AudioSourceHandler src_handler;
  beat::WaveformGeneration wave_gen;
  
#ifdef _MSC_VER
  std::string wk_dir = "../sounds/";
#else
  std::string wk_dir = "./sounds/";
#endif
  std::cout << std::filesystem::current_path().string() << std::endl;
  auto wd_cat = beat::WaveformIO::load(wk_dir + "Cat_Meow_2-Cat_Stevens-2034822903.wav", 3);
  if (!wd_cat.empty())
  {
    auto src_cat = src_handler.create_source_from_waveform(wd_cat);
    std::cout << "Cat (channels): "<< wd_cat.size() << std::endl;
    std::cout << "Cat (samples): " << wd_cat[0].buffer.size() << std::endl;
    src_cat->play(beat::PlaybackMode::STATE_WAIT);

    std::cout << "---\n";
    beat::WaveformIO::save(wd_cat, wk_dir + "cat meow saved.wav", beat::AudioFileFormatSubType::PCM_16, 3);
    
    // Reverb
    
    bool load_and_play_reverb = false; // If false, creates reverb and saves it to file.
    
    if (!load_and_play_reverb)
    {
      std::cout << "---\n";
      auto wd_rev = beat::WaveformIO::load(wk_dir + "07a-AirRaidShelterCentre-DPA 4061-Stereo-48K-BURST.wav", 3);
      if (!wd_rev.empty())
      {
        std::cout << "Reverb kernel (channels): "<< wd_rev.size() << std::endl;
        auto wd_cat_in_air_raid_shelter = beat::WaveformHelper::apply_channelwise(beat::WaveformHelper::reverb_fast, wd_cat, wd_rev);
        auto src_cat_in_air_raid_shelter = src_handler.create_source_from_waveform(wd_cat_in_air_raid_shelter);
        std::cout << "Cat in Air Raid Shelter (channels): " << wd_cat_in_air_raid_shelter.size() << std::endl;
        std::cout << "Cat in Air Raid Shelter (samples): " << wd_cat_in_air_raid_shelter[0].buffer.size() << std::endl;
        src_cat_in_air_raid_shelter->play(beat::PlaybackMode::STATE_WAIT);
        std::cout << "---\n";
        beat::WaveformIO::save(wd_cat_in_air_raid_shelter, wk_dir + "cat meow reverb saved.wav", beat::AudioFileFormatSubType::PCM_16, 3);
      }
    }
    else
    {
      std::cout << "---\n";
      auto wd_cat_in_air_raid_shelter = beat::WaveformIO::load(wk_dir + "cat meow reverb saved.wav", 3);
      if (!wd_cat_in_air_raid_shelter.empty())
      {
        auto src_cat_in_air_raid_shelter = src_handler.create_source_from_waveform(wd_cat_in_air_raid_shelter);
        src_cat_in_air_raid_shelter->play(beat::PlaybackMode::STATE_WAIT);
      }
    }
  }
 
  keyboard.pressAnyKey();
  
  return 0;
}
