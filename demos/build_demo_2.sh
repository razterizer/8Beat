#!/bin/bash

os_name=$(uname)

if [[ $os_name == *"Darwin"* ]]; then
  additional_flags="-I/opt/homebrew/opt/openal-soft/include -I/opt/homebrew/opt/libsndfile/include \
  -L/opt/homebrew/opt/openal-soft/lib -L/opt/homebrew/opt/libsndfile/lib \
  -lopenal -lsndfile"
else
  additional_flags=""
  # #FIXME: Add libsndfile here as well.
  export BUILD_PKG_CONFIG_MODULES='openal'
fi

../../Core/build.sh demo_2 "$1" "${additional_flags[@]}"
