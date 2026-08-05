# Release notes

## Next

- Pin the OpenAL source profile to the adapter fix that keeps non-spatial
  sources listener-relative when the listener moves.

## 1.0.3.6

- Keep command separators out of waveform generation and playback.
- Reject empty or invalid audio buffers before passing them to the selected
  backend.
- Keep asynchronous tune playback joinable for safe stopping and destruction.
- Restrict hosted release and cbox publication to the MIT-licensed applaudio
  backend; OpenAL remains available through local and pinned Git sources.

## 1.0.3.5

- Bumped AudioLibSwitcher_OpenAL dependency version.

## 1.0.2.4

- Transitive dependency to applaudio now has the one with the windows lib "ksguid" addition.

## 1.0.1.3

- Removed libsndfile from Forge-published dependency metadata and cbox headers.
  `WaveformIO.h` and `demo_2` remain source-tree examples for manual libsndfile
  builds, but are excluded from standard Forge release artifacts.

## 1.0.1.2

- Publish Forge cbox variants for audio backend selection.
- Disable OpenAL-based release publishing; public Forge release assets now use the applaudio variant.

## 1.0.0.1

- Initial release.
