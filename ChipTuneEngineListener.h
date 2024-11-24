//
//  IChipTuneEngineListener.h
//  8Beat
//
//  Created by Rasmus Anthin on 2024-11-23.
//

#pragma once
#include <Core/IListener.h>
#include <string>
#include <optional>


struct ChipTuneEngineListener : IListener
{
  virtual std::optional<std::string> on_tune_ended(const std::string& /*curr_tune_filepath*/) = 0;
};
