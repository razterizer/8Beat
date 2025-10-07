//
//  ChipTuneEngine.h
//  8Beat
//
//  Created by Rasmus Anthin on 2024-01-21.
//
#pragma once

#include "ChipTuneEngine_Internals/ChipTuneEngineParser.h"
#include "ChipTuneEngineListener.h"
#include "AudioSourceHandler.h"
#include "Waveform.h"
#include "WaveformGeneration.h"

#include <Core/Delay.h>
#include <Core/StringHelper.h>
#include <Core/events/EventBroadcaster.h>

#include <vector>
#include <chrono>
#include <thread>
#include <atomic>


namespace beat
{

  class ChipTuneEngine : public ChipTuneEngineParser, public EventBroadcaster<ChipTuneEngineListener>
  {
  public:
    ChipTuneEngine(AudioSourceHandler& audio_handler, const WaveformGeneration& waveform_gen)
      : ChipTuneEngineParser(audio_handler, waveform_gen)
    {}
    ~ChipTuneEngine() = default;

    // Play the loaded tune
    bool play_tune(bool interrupt_unfinished_note = true, bool verbose = false)
    {
      if (m_voices.empty())
        return false; // No tune loaded.
    
      if (verbose)
        std::cout << "Playing Tune" << std::endl;
      
      auto f_find_label = [this](const auto& label)
      {
        return std::find_if(m_labels.begin(), m_labels.end(), [&label](const auto& lp) { return lp.second->label == label; });
      };
      
      auto f_reset_volume = [this](int goto_note_idx)
      {
        for (const auto& vol_pair : m_volume)
          if (vol_pair.first <= goto_note_idx)
            m_curr_volume = vol_pair.second;
      };
      auto f_reset_speed = [this](int goto_note_idx)
      {
        for (const auto& ts_pair : m_time_step_ms)
          if (ts_pair.first <= goto_note_idx)
            m_curr_time_step_ms = ts_pair.second;
      };
      
      if (auto it_ts = m_time_step_ms.find(0); it_ts != m_time_step_ms.end())
        m_curr_time_step_ms = it_ts->second;
      m_curr_volume = 1.f;
      if (auto it_v = m_volume.find(0); it_v != m_volume.end())
        m_curr_volume = it_v->second;
        
      Delay::sleep(static_cast<int>(1e6f)); // Warm-up. #FIXME: Find a better, more robust solution.
      // ### Loop over voices ###
      auto num_notes = static_cast<int>(m_voices[0].notes.size());
      for (int note_idx = note_start_idx; note_idx < num_notes; ++note_idx)
      {
        if (m_stop_audio_thread)
          break;
          
        if (auto it_p = m_print_switches.find(note_idx); it_p != m_print_switches.end())
        {
          if (it_p->second)
            enable_print_notes();
          else
            disable_print_notes();
        }
      
        // Branching.
        if (auto it_g = m_gotos.find(note_idx); it_g != m_gotos.end())
        {
          auto& goto_data = *it_g->second.get();
          const auto& from_label = goto_data.from_label;
          const auto& to_label = goto_data.to_label;
          auto& count = goto_data.count;
          
          if (m_enable_print_notes)
          {
            if (from_label == "GOTO")
              std::cout << from_label << " " << to_label << std::endl;
            else if (from_label == "GOTO_TIMES")
              std::cout << from_label << " " << to_label << " " << count << std::endl;
            else if (count == -2)
              std::cout << from_label << std::endl;
          }
          
          if (from_label == "DA_CAPO_AL_FINE")
          {
            m_al_fine = true;
            note_idx = -1;
            continue;
          }
          else if (from_label == "DA_CAPO_AL_CODA")
          {
            m_al_coda = true;
            note_idx = -1;
            continue;
          }
          else if (from_label == "DAL_SEGNO_AL_FINE")
          {
            if (auto it = f_find_label("SEGNO"); it != m_labels.end())
            {
              m_al_fine = true;
              note_idx = it->first - 1;
              f_reset_volume(note_idx);
              f_reset_speed(note_idx);
              continue;
            }
          }
          else if (from_label == "DAL_SEGNO_AL_CODA")
          {
            if (auto it = f_find_label("SEGNO"); it != m_labels.end())
            {
              m_al_coda = true;
              note_idx = it->first - 1;
              f_reset_volume(note_idx);
              f_reset_speed(note_idx);
              continue;
            }
          }
          else if (m_al_coda && from_label == "TO_CODA")
          {
            m_al_coda = false;
            m_to_coda = true;
            auto it_l = f_find_label("CODA");
            if (it_l != m_labels.end())
            {
              note_idx = it_l->first - 1;
              f_reset_volume(note_idx);
              f_reset_speed(note_idx);
              continue;
            }
          }
          
          // Goto label.
          if (count > 0 || count == -1)
          {
            if (count > 0)
              count--;
            
            auto it_l = f_find_label(to_label);
            if (it_l != m_labels.end())
            {
              note_idx = it_l->first - 1;
              continue;
            }
          }
          
          if (count == 0)
            goto_data.reset();
        }
        
        if (m_enable_print_notes)
        {
          for (auto it = m_labels.begin(); it != m_labels.end(); ++it)
            if (it->first == note_idx)
            {
              if (it->second->label == "ENDING")
                std::cout << it->second->label << " " << it->second->id << std::endl;
              else if (it->second->id == 0)
                std::cout << "LABEL " << it->second->label << std::endl;
              else if (it->second->id == -2)
                std::cout << it->second->label << std::endl;
            }
        }
        
        // Special labels.
        if (m_al_fine)
        {
          if (auto it = f_find_label("FINE"); it != m_labels.end() && it->first == note_idx)
          {
            m_al_fine = false;
            break;
          }
        }
        else if (m_to_coda)
        {
          if (auto it = f_find_label("CODA"); it != m_labels.end() && it->first == note_idx)
            m_to_coda = false;
        }

        // N:th ending.
        if (auto it = stlutils::find_if(m_labels, [note_idx](const auto& lp) { return lp.first == note_idx && lp.second->label == "ENDING"; });
            it != m_labels.end())
        {
          auto* lbl = it->second.get();
          if (lbl->src_goto != nullptr)
          {
            // If we are standing at an ENDING for the following repetition(s).
            int curr_num_repeats = lbl->src_goto->num_jumps();
            if (curr_num_repeats < lbl->id)
            {
              note_idx = lbl->src_goto->note_idx - 1;
              continue;
            }
            else if (curr_num_repeats > lbl->id)
            {
              auto it_rlp = stlutils::find_if(lbl->related_labels, [curr_num_repeats](const auto& rlp)
              {
                return rlp.second->id == curr_num_repeats;
              });
              if (it_rlp != lbl->related_labels.end())
              {
                int rl_note_idx = it_rlp->first;
                note_idx = rl_note_idx - 1;
                continue;
              }
            }
          }
        }
      
        // Volume.
        if (auto it_v = m_volume.find(note_idx); it_v != m_volume.end())
          m_curr_volume = it_v->second;
        
        // The Melody.
        bool is_separator = m_voices[0].notes[note_idx].get()->separator;
        for (const auto& voice : m_voices)
        {
          auto* note = voice.notes[note_idx].get();
          if (voice.src != nullptr)
          {
            if (!note->pause && (interrupt_unfinished_note || !voice.src->is_playing()))
            {
              if (interrupt_unfinished_note)
                voice.src->stop();
              if (m_ir_sound != nullptr)
              {
                auto wd_rev = WaveformHelper::reverb_fast(note->wave, *m_ir_sound);
                voice.src->update_buffer(wd_rev);
              }
              else
                voice.src->update_buffer(note->wave);
              voice.src->set_volume(m_ext_volume * m_curr_volume * note->volume);
              voice.src->play(PlaybackMode::NONE);
            }
          }
        }
        
        if (m_enable_print_notes)
          std::cout << "Note Idx: " << std::to_string(note_idx) << std::endl;
                                      
        // Tempo.
        if (auto it_ts = m_time_step_ms.find(note_idx); it_ts != m_time_step_ms.end())
          m_curr_time_step_ms = it_ts->second;
        if (!is_separator)
          Delay::sleep(static_cast<int>(m_curr_time_step_ms*1e3f));
        
        do {}
        while (m_pause);
      }

      // Cooldown.
      do {}
      while (stlutils::contains_if(m_voices, [](const auto& voice) { return voice.src->is_playing(); }));
      
      broadcast([this](auto* listener) { listener->on_tune_ended(this, m_curr_file_path); });
      
      return true;
    }
    
    // Play the loaded tune in a separate thread
    void play_tune_async(bool interrupt_unfinished_note = true, bool verbose = false)
    {
      // Use std::thread and std::atomic_flag to safely start and stop the thread
      m_stop_audio_thread = false;
      m_audio_thread = std::thread([this, interrupt_unfinished_note, verbose] { play_tune(interrupt_unfinished_note, verbose); });
    
      // Detach the audio thread, allowing it to run independently
      m_audio_thread.detach();
    }

    // Stop the audio playback thread
    void stop_tune_async()
    {
      m_stop_audio_thread = true;

      // Optionally, you can join the thread here if you want to wait for it to finish
      if (m_audio_thread.joinable())
        m_audio_thread.join();
    }

    // Wait for the audio playback thread to finish
    void wait_for_completion()
    {
      if (m_audio_thread.joinable())
        m_audio_thread.join();
    }
    
    void enable_print_notes()
    {
      m_enable_print_notes = true;
    }
    
    void disable_print_notes()
    {
      m_enable_print_notes = false;
    }
    
    void pause()
    {
      m_pause = true;
    }
    
    void resume()
    {
      m_pause = false;
    }
    
    void set_volume(float vol)
    {
      m_ext_volume = vol;
    }
    
    // #WARNING: Super-slow!!!
    void set_reverb_ir(const Waveform* ir)
    {
      m_ir_sound = ir;
    }
    
    void reset_reverb()
    {
      m_ir_sound = nullptr;
    }
    
  private:
    std::thread m_audio_thread;
    std::atomic<bool> m_stop_audio_thread = false;
    std::atomic<bool> m_pause = false;
    std::atomic<bool> m_enable_print_notes = false;
    std::atomic<float> m_ext_volume = 1.f;
    std::atomic<Waveform const *> m_ir_sound = nullptr;
    std::atomic<bool> m_use_reverb = false;
  };

}
