#!/bin/bash

os_name=$(uname)

if [[ $os_name == *"Darwin"* ]]; then
  additional_flags="\
    -I../include/8Beat \
    -I../../Core/include \
    -I../../Termin8or/include \
    -I../../TrainOfThought/include \
    -I../../AudioLibSwitcher/include \
    -I../../AudioLibSwitcher_OpenAL/include \
    -I/opt/homebrew/opt/openal-soft/include -L/opt/homebrew/opt/openal-soft/lib -lopenal"
else
  additional_flags="\
    -I../include/8Beat \
    -I../../Core/include \
    -I../../Termin8or/include \
    -I../../TrainOfThought/include \
    -I../../AudioLibSwitcher/include \
    -I../../AudioLibSwitcher_OpenAL/include"
  export BUILD_PKG_CONFIG_MODULES='openal'
fi

../../Core/build.sh demo_7 "$1" "${additional_flags[@]}"

# Capture the exit code of Core/build.sh
exit_code=$?

if [ $exit_code -ne 0 ]; then
  echo "Core/build.sh failed with exit code $exit_code"
  exit $exit_code
fi
