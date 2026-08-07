#pragma once

#include <8Beat/ChipTuneEngine.h>
#include <8Beat/ChipTuneEngineListener.h>

#include <atomic>
#include <cassert>
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

namespace beat
{
  class ReplayOnceListener final : public ChipTuneEngineListener
  {
  public:
    explicit ReplayOnceListener(std::string tune_filepath)
      : m_tune_filepath(std::move(tune_filepath))
    {}

    void on_tune_ended(ChipTuneEngine* engine, const std::string&) override
    {
      const auto completion_count = ++m_completion_count;
      if (completion_count != 1)
        return;

      m_reload_succeeded = engine->load_tune(m_tune_filepath);
      if (m_reload_succeeded)
        engine->play_tune_async();
    }

    std::atomic<int> m_completion_count = 0;
    std::atomic<bool> m_reload_succeeded = false;

  private:
    std::string m_tune_filepath;
  };

  inline void chiptune_engine_unit_tests(const std::string& tune_filepath)
  {
    std::ostringstream backend_errors;
    auto* original_error_buffer = std::cerr.rdbuf(backend_errors.rdbuf());

    AudioSourceHandler audio_handler(false);
    WaveformGeneration waveform_generation;
    ChipTuneEngine engine(audio_handler, waveform_generation);
    ReplayOnceListener listener(tune_filepath);

    engine.add_listener(&listener);
    assert(engine.load_tune(tune_filepath));
    engine.play_tune_async();
    engine.wait_for_completion();
    engine.remove_listener(&listener);

    assert(listener.m_reload_succeeded);
    assert(listener.m_completion_count == 2);

    assert(engine.load_tune(tune_filepath));
    engine.pause();
    engine.play_tune_async();
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    engine.stop_tune_async();

    const auto audio_error = m_audio_lib.check_error();
    if (!audio_error.empty())
      std::cerr << "Audio backend error after chained tune cleanup: " << audio_error << '\n';

    std::cerr.rdbuf(original_error_buffer);
    if (!backend_errors.str().empty())
      std::cerr << backend_errors.str();
    assert(backend_errors.str().empty());
    assert(audio_error.empty());
  }
}
