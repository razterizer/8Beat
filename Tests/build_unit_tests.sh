#!/bin/bash


os_name=$(uname)

additional_flags="-I../.. \
  -I../include \
  -I../include/8Beat \
  -I../../Core/include \
  -I../../AudioLibSwitcher_applaudio/include \
  -I../../AudioLibSwitcher_applaudio/AudioLibSwitcher/include \
  -I../../applaudio/include \
  -DUSE_APPLAUDIO"

if [[ $os_name == *"Darwin"* ]]; then
  additional_flags+=" -framework AudioToolbox -framework CoreAudio -framework CoreFoundation"
else
  export BUILD_PKG_CONFIG_MODULES='alsa'
fi

../../Core/build.sh unit_tests "$1" "${additional_flags[@]}"

# Capture the exit code of Core/build.sh
exit_code=$?

if [ $exit_code -ne 0 ]; then
  echo "Core/build.sh failed with exit code $exit_code"
  exit $exit_code
fi

cp fixtures/async_chain.ct bin/async_chain.ct
