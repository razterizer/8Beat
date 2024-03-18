#!/bin/bash

additional_flags="-I/opt/homebrew/opt/openal-soft/include -I/opt/homebrew/opt/libsndfile/include \
  -L/opt/homebrew/opt/openal-soft/lib -L/opt/homebrew/opt/libsndfile/lib \
  -lopenal -lsndfile"
#additional_flags="-I/opt/homebrew/opt/openal-soft/include -L/opt/homebrew/opt/openal-soft/lib -lopenal"
#additional_flags="-I/opt/homebrew/opt/openal-soft/include -L/opt/homebrew/opt/openal-soft/lib -lopenal.1.23.1"
#additional_flags="-I/opt/homebrew/opt/openal-soft/include -L/opt/homebrew/opt/openal-soft/lib /opt/homebrew/opt/openal-soft/lib/libopenal.dylib"


../../Core/build.sh demo_8 $1 "${additional_flags[@]}"
