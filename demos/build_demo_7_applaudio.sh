#!/bin/bash

os_name=$(uname)

if [[ $os_name == *"Darwin"* ]]; then
  additional_flags="\
    -I../include/8Beat \
    -I../../Core/include \
    -I../../Termin8or/include \
    -I../../TrainOfThought/include \
    -I../../AudioLibSwitcher_applaudio/include \
    -I../../applaudio/include -framework AudioToolbox -framework CoreAudio -framework CoreFoundation -DUSE_APPLAUDIO"
else
  additional_flags="\
    -I../include/8Beat \
    -I../../Core/include \
    -I../../Termin8or/include \
    -I../../TrainOfThought/include \
    -I../../AudioLibSwitcher_applaudio/include \
    -I../../applaudio/include -DUSE_APPLAUDIO"
  export BUILD_PKG_CONFIG_MODULES='alsa'
fi

../../Core/build.sh demo_7 "$1" "${additional_flags[@]}"

# Capture the exit code of Core/build.sh
exit_code=$?

if [ $exit_code -ne 0 ]; then
  echo "Core/build.sh failed with exit code $exit_code"
  exit $exit_code
fi
