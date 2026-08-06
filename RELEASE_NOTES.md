# Release notes

## Next

- Allow tune-ended listeners to queue another asynchronous tune without the
  playback thread attempting to join itself.
- Allow a paused playback thread to stop cleanly and suppress tune-ended
  callbacks for cancelled playback.
- Destroy audio sources before their buffers so OpenAL can release attached
  buffers cleanly between chained tunes.
- Release voice sources when replacing a loaded tune.
- Pin the OpenAL source profile to adapter release 1.0.1.14, which keeps
  non-spatial sources listener-relative when the listener moves.

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
