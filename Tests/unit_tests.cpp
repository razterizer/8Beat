//
//  unit_tests.cpp
//  8-Bit Audio Emulator Lib
//
//  Created by Rasmus Anthin on 2024-09-29.
//

#include "WaveformHelper_tests.h"
#include "ChipTuneEngine_tests.h"
#include <filesystem>
#include <iostream>


int main(int argc, char** argv)
{
  std::cout << "### WaveformHelper Tests ###" << std::endl;
  beat::unit_tests();

  std::cout << "### ChipTuneEngine Tests ###" << std::endl;
  beat::chiptune_engine_unit_tests(
    (std::filesystem::path(argv[0]).parent_path() / "async_chain.ct").string());
  
  return 0;
}
