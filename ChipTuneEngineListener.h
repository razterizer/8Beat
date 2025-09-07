//
//  IChipTuneEngineListener.h
//  8Beat
//
//  Created by Rasmus Anthin on 2024-11-23.
//

#pragma once
#include <Core/events/IListener.h>
#include <string>
#include <optional>

namespace beat
{
  class ChipTuneEngine;
  
  
  struct ChipTuneEngineListener : IListener
  {
    virtual void on_tune_ended(ChipTuneEngine* /*engine*/, const std::string& /*curr_tune_filepath*/) = 0;
  };
  
}
