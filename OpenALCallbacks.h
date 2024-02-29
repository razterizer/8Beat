//
//  OpenALCallbacks.h
//  8-Bit Audio Emulator Lib
//
//  Created by Rasmus Anthin on 2024-02-28.
//

#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <iostream>

#define alCall(function, ...) alCallImpl(__FILE__, __LINE__, function, __VA_ARGS__)
#define alcCall(function, device, ...) alcCallImpl(__FILE__, __LINE__, function, device, __VA_ARGS__)

void check_al_errors(const std::string& filename, const std::uint_fast32_t line)
{
  ALCenum error = alGetError();
  if(error != AL_NO_ERROR)
  {
    std::cerr << "***ERROR*** (" << filename << ": " << line << ")\n" ;
    switch(error)
    {
      case AL_INVALID_NAME:
        std::cerr << "AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL function";
        break;
      case AL_INVALID_ENUM:
        std::cerr << "AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL function";
        break;
      case AL_INVALID_VALUE:
        std::cerr << "AL_INVALID_VALUE: an invalid value was passed to an OpenAL function";
        break;
      case AL_INVALID_OPERATION:
        std::cerr << "AL_INVALID_OPERATION: the requested operation is not valid";
        break;
      case AL_OUT_OF_MEMORY:
        std::cerr << "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory";
        break;
      default:
        std::cerr << "UNKNOWN AL ERROR: " << error;
    }
    std::cerr << std::endl;
  }
}

template<typename alFunction, typename... Params>
auto alCallImpl(const char* filename, const std::uint_fast32_t line, alFunction function, Params... params)
->typename std::enable_if<std::is_same<void,decltype(function(params...))>::value,decltype(function(params...))>::type
{
  function(std::forward<Params>(params)...);
  check_al_errors(filename,line);
}

template<typename alFunction, typename... Params>
auto alCallImpl(const char* filename, const std::uint_fast32_t line, alFunction function, Params... params)
->typename std::enable_if<!std::is_same<void,decltype(function(params...))>::value,decltype(function(params...))>::type
{
  auto ret = function(std::forward<Params>(params)...);
  check_al_errors(filename,line);
  return ret;
}
