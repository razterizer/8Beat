#!/bin/bash

os_name=$(uname)

if [[ $os_name == *"Darwin"* ]]; then
  additional_flags="-I../.. \
  -I/opt/homebrew/opt/openal-soft/include -L/opt/homebrew/opt/openal-soft/lib -lopenal"
else
  additional_flags=""
  export BUILD_PKG_CONFIG_MODULES='openal'
fi

../../Core/build.sh demo_10 "$1" "${additional_flags[@]}"
