//
//  demo_2.cpp
//  8Beat
//
//  Created by Rasmus Anthin on 2024-03-16.
//

#include "../AudioSourceHandler.h"
#include "../WaveformGeneration.h"
#include "../WaveformHelper.h"
#include "../WaveformIO.h"
#include <Termin8or/Keyboard.h>
#include <filesystem>


int main(int argc, char** argv)
{
  t8::input::StreamKeyboard keyboard;

  audio::AudioSourceHandler src_handler;
  audio::WaveformGeneration wave_gen;
  
#ifdef _MSC_VER
  std::string wk_dir = "../sounds/";
#else
  std::string wk_dir = "./sounds/";
#endif
  std::cout << std::filesystem::current_path().string() << std::endl;
  auto wd_cat_ld = audio::WaveformIO::load(wk_dir + "Cat_Meow_2-Cat_Stevens-2034822903.wav", 3);
  if (wd_cat_ld.has_value())
  {
    const auto& wd_cat = wd_cat_ld.value();
    auto src_cat = src_handler.create_source_from_waveform(wd_cat);
    std::cout << "Cat (samples): " << wd_cat.buffer.size() << std::endl;
    src_cat->play(audio::PlaybackMode::STATE_WAIT);

    std::cout << "---\n";
    audio::WaveformIO::save(wd_cat, wk_dir + "cat meow saved.wav", audio::AudioFileFormatSubType::PCM_16, 3);
    
    // Reverb
    
    bool load_and_play_reverb = false; // If false, creates reverb and saves it to file.
    
    if (!load_and_play_reverb)
    {
      std::cout << "---\n";
      auto wd_rev_ld = audio::WaveformIO::load(wk_dir + "07a-AirRaidShelterCentre-DPA 4061-Stereo-48K-BURST.wav", 3);
      if (wd_rev_ld.has_value())
      {
        auto& wd_rev = wd_rev_ld.value();
        auto wd_cat_in_air_raid_shelter = audio::WaveformHelper::reverb_fast(wd_cat, wd_rev);
        auto src_cat_in_air_raid_shelter = src_handler.create_source_from_waveform(wd_cat_in_air_raid_shelter);
        std::cout << "Cat in Air Raid Shelter (samples): " << wd_cat_in_air_raid_shelter.buffer.size() << std::endl;
        src_cat_in_air_raid_shelter->play(audio::PlaybackMode::STATE_WAIT);
        std::cout << "---\n";
        audio::WaveformIO::save(wd_cat_in_air_raid_shelter, wk_dir + "cat meow reverb saved.wav", audio::AudioFileFormatSubType::PCM_16, 3);
      }
    }
    else
    {
      std::cout << "---\n";
      auto wd_cat_in_air_raid_shelter_ld = audio::WaveformIO::load(wk_dir + "cat meow reverb saved.wav", 3);
      if (wd_cat_in_air_raid_shelter_ld.has_value())
      {
        const auto& wd_cat_in_air_raid_shelter = wd_cat_in_air_raid_shelter_ld.value();
        auto src_cat_in_air_raid_shelter = src_handler.create_source_from_waveform(wd_cat_in_air_raid_shelter);
        src_cat_in_air_raid_shelter->play(audio::PlaybackMode::STATE_WAIT);
      }
    }
  }
 
  keyboard.pressAnyKey();
  
  return 0;
}
