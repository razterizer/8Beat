//
//  ChipTuneEngine.h
//  8-Bit Audio Emulator Lib
//
//  Created by Rasmus Anthin on 2024-01-21.
//
#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>
#include "AudioSourceHandler.h"
#include "Waveform.h"
#include "WaveformGeneration.h"
#include "../Terminal Text Lib/Delay.h"
#include "../Core Lib/StringHelper.h"
#include <thread>
#include <atomic>

namespace audio
{

  class ChipTuneEngine
  {
  public:
    ChipTuneEngine(AudioSourceHandler& audio_handler, const WaveformGeneration& waveform_gen)
      : m_audio_handler(audio_handler)
      , m_waveform_gen(waveform_gen)
    {
    }
    ~ChipTuneEngine()
    {
      for (const auto& voice : m_voices)
        m_audio_handler.remove_source(voice.src);
    }

    // Load tune from a text file with a specific format
    bool load_tune(const std::string& filePath)
    {
      std::ifstream file(filePath);
      if (!file.is_open())
      {
        std::cerr << "Error opening tune file: " << filePath << std::endl;
        return false;
      }

      std::cout << "Parsing Tune" << std::endl;
      std::string line;
      while (std::getline(file, line))
      {
        parse_line(line);
      }
      
      std::cout << "Creating Instruments" << std::endl;
      create_instruments();
      std::cout << "Initializing Sources" << std::endl;
      init_voice_sources();

      return true;
    }

    // Play the loaded tune
    void play_tune()
    {
      std::cout << "Playing Tune" << std::endl;
      auto num_notes = static_cast<int>(m_voices[0].notes.size());
      for (int note_idx = 0; note_idx < num_notes; ++note_idx)
      {
        for (const auto& voice : m_voices)
        {
          auto* note = voice.notes[note_idx].get();
          if (voice.src != nullptr)
          {
            // #FIXME: Use streaming audio source.
            if (!note->pause && !voice.src->is_playing())
            {
              voice.src->update_buffer(note->wave);
              voice.src->set_volume(note->volume);
              voice.src->play(PlaybackMode::NONE);
            }
          }
        }
        Delay::sleep(time_step_ms*1e3f);
      }
    }
    
    // Play the loaded tune in a separate thread
    void play_tune_async()
    {
      // Use std::thread and std::atomic_flag to safely start and stop the thread
      stop_audio_thread = false;
      audio_thread = std::thread([this] { play_tune(); });
    
      // Detach the audio thread, allowing it to run independently
      audio_thread.detach();
    }

    // Stop the audio playback thread
    void stop_tune_async()
    {
      stop_audio_thread = true;

      // Optionally, you can join the thread here if you want to wait for it to finish
      if (audio_thread.joinable())
        audio_thread.join();
    }

    // Wait for the audio playback thread to finish
    void wait_for_completion()
    {
      if (audio_thread.joinable())
        audio_thread.join();
    }

  private:
    struct Note
    {
      Note() : pause(true) {}
      Note(float f, float d) : pause(false), frequency(f), duration_ms(d) {}
      bool pause = false;
      float frequency = 0.f;
      float duration_ms = 0.f;
      Waveform wave;
      int instrument_basic_idx = -1;
      int instrument_ring_mod_idx = -1;
      int instrument_weight_avg_idx = -1;
      int instrument_lib_idx = -1;
      float volume = 1.f;
    };
    struct InstrumentBasic
    {
      std::string name;
      WaveformType waveform = WaveformType::SINE_WAVE;
      float duty_cycle = 0.5f;
      int adsr_idx = -1;
      int flp_idx = -1;
      float volume = 1.f;
    };
    struct InstrumentRingMod
    {
      std::string name;
      std::string ring_mod_instr_name_A;
      std::string ring_mod_instr_name_B;
      int adsr_idx = -1;
      int flp_idx = -1;
      float volume = 1.f;
    };
    struct InstrumentWeightAvg
    {
      std::string name;
      std::vector<std::pair<float, std::string>> instrument_names;
      int adsr_idx = -1;
      int flp_idx = -1;
      float volume = 1.f;
    };
    struct InstrumentLib
    {
      std::string name;
      InstrumentType lib_instrument = InstrumentType::PIANO;
      FrequencyType freq_effect = FrequencyType::CONSTANT;
      AmplitudeType ampl_effect = AmplitudeType::CONSTANT;
      PhaseType phase_effect = PhaseType::ZERO;
      float volume = 1.f;
    };
    struct Voice
    {
      AudioStreamSource* src = nullptr;
      std::vector<std::unique_ptr<Note>> notes;
    };

    AudioSourceHandler& m_audio_handler;
    const WaveformGeneration& m_waveform_gen;
    std::vector<InstrumentBasic> m_instruments_basic;
    std::vector<InstrumentRingMod> m_instruments_ring_mod;
    std::vector<InstrumentWeightAvg> m_instruments_weight_avg;
    std::vector<InstrumentLib> m_instruments_lib;
    std::vector<ADSR> m_envelopes;
    std::vector<LowPassFilterArgs> m_filter_lp_args;
    float time_step_ms = 100;
    int num_voices = 0;
    std::vector<Voice> m_voices;
    //std::vector<Instrument> m_instruments;

    void parse_line(const std::string& line)
    {
      std::istringstream iss(line);
      if (line.find("instrument") == 0
        || line.find("adsr") == 0
        || line.find("filter_lp") == 0
        || line.find("TIME_STEP_MS") == 0
        || line.find("NUM_VOICES") == 0
        || line.find("TAB") == 0)
      {
        std::string command;
        if (iss >> command)
        {
          if (command == "instrument")
            parse_instrument(line, iss);
          else if (command == "adsr")
            parse_envelopes(line, iss);
          else if (command == "filter_lp")
            parse_lp_filters(line, iss);
          else if (command == "TIME_STEP_MS")
            iss >> time_step_ms;
          else if (command == "NUM_VOICES")
          {
            iss >> num_voices;
            if (num_voices > 0)
              m_voices.resize(num_voices);
          }
          else if (command == "TAB")
            parse_tab(line, iss);
        }
        else
          std::cerr << "Error parsing instrument line: " << line << std::endl;
      }
    }
    
    void parse_instrument(const std::string& line, std::istringstream& iss)
    {
      std::string instrument_name, waveform_name, modifier;
      std::string op;
      float duty_cycle = 0.5f, vol = 1.f;
      int adsr_nr = -1, flp_nr = -1;
      
      iss >> instrument_name;

      std::string unread = iss.eof()  ?  "" : iss.str().substr(iss.tellg());
      op = str::ltrim_ret(unread);
      
      if (op.find("(") == 0)
      {
        // Weighted average.
        auto idx = op.find("adsr");
        idx = idx != std::string::npos ? idx - 1 : op.length();
        math::minimize(idx, op.find("flp"));
        idx = idx != std::string::npos ? idx - 1 : op.length();
        std::string weighted_sum = op.substr(0, idx);
        str::remove_spaces(weighted_sum);
        auto& instrument = m_instruments_weight_avg.emplace_back();
        instrument.name = instrument_name;
        if (!weighted_sum.empty() && weighted_sum[0] == '(')
        {
          weighted_sum.erase(0, 1);
          while (!weighted_sum.empty() && weighted_sum[0] == '(')
          {
            float weight = 0.f;
            std::string instr;
            weighted_sum.erase(0, 1);
            auto comma_idx = weighted_sum.find(',');
            if (!(std::istringstream(weighted_sum.substr(0, comma_idx)) >> weight))
              std::cerr << "Error parsing weight in instrument line: \"" << line << "\"." << std::endl;
            weighted_sum.erase(0, comma_idx + 1);
            auto rp_idx = weighted_sum.find(')');
            if (!(std::istringstream(weighted_sum.substr(0, rp_idx)) >> instr))
              std::cerr << "Error parsing instrument name in instrument line: \"" << line << "\"." << std::endl;
            weighted_sum.erase(0, rp_idx + 1);
            if (weighted_sum[0] == ',')
              weighted_sum.erase(0, 1);
            instrument.instrument_names.emplace_back(weight, instr);
          }
          if (weighted_sum.find(')') != std::string::npos)
            weighted_sum.erase(0, 1);
          else
            std::cerr << "Error: Missing right parenthesis in instrument line: \"" << line << "\"." << std::endl;
        }
        
        while (iss >> modifier)
        {
          auto col_idx = modifier.find(':');
          if (col_idx != std::string::npos)
          {
            auto modifier_name = modifier.substr(0, col_idx);
            auto modifier_val = modifier.substr(col_idx + 1);
            
            if (modifier_name == "adsr")
            {
              if (!(std::istringstream(modifier_val) >> adsr_nr))
                std::cerr << "Error parsing adsr in instrument line: \"" << line << "\"." << std::endl;
            }
            else if (modifier_name == "flp")
            {
              if (!(std::istringstream(modifier_val) >> flp_nr))
                std::cerr << "Error parsing flp in instrument line: \"" << line << "\"." << std::endl;
            }
            else if (modifier_name == "vol")
            {
              if (!(std::istringstream(modifier_val) >> vol))
                std::cerr << "Error parsing vol in instrument line: \"" << line << "\"." << std::endl;
            }
          }
        }
        instrument.adsr_idx = adsr_nr;
        instrument.flp_idx = flp_nr;
        instrument.volume = vol;
      }
      else if (op.find("&") == 0)
      {
        std::string lib_instrument, freq_effect, ampl_effect, phase_effect;
        // Library instrument.
        iss >> lib_instrument >> freq_effect >> ampl_effect >> phase_effect;
        lib_instrument.erase(0, 1);
        
        auto& instr = m_instruments_lib.emplace_back();
        instr.name = instrument_name;

        if (lib_instrument == "PIANO")
          instr.lib_instrument = InstrumentType::PIANO;
        else if (lib_instrument == "VIOLIN")
          instr.lib_instrument = InstrumentType::VIOLIN;
        else if (lib_instrument == "ORGAN")
          instr.lib_instrument = InstrumentType::ORGAN;
        else if (lib_instrument == "TRUMPET")
          instr.lib_instrument = InstrumentType::TRUMPET;
        else if (lib_instrument == "FLUTE")
          instr.lib_instrument = InstrumentType::FLUTE;
        else if (lib_instrument == "KICKDRUM")
          instr.lib_instrument = InstrumentType::KICKDRUM;
        else if (lib_instrument == "SNAREDRUM")
          instr.lib_instrument = InstrumentType::SNAREDRUM;
        else if (lib_instrument == "HIHAT")
          instr.lib_instrument = InstrumentType::HIHAT;
        else if (lib_instrument == "ANVIL")
          instr.lib_instrument = InstrumentType::ANVIL;
        
        if (freq_effect.empty() || freq_effect == "CONSTANT")
          instr.freq_effect = FrequencyType::CONSTANT;
        else if (freq_effect == "JET_ENGINE_POWERUP")
          instr.freq_effect = FrequencyType::JET_ENGINE_POWERUP;
        
        if (ampl_effect.empty() || ampl_effect == "CONSTANT")
          instr.ampl_effect = AmplitudeType::CONSTANT;
        else if (ampl_effect == "JET_ENGINE_POWERUP")
          instr.ampl_effect = AmplitudeType::JET_ENGINE_POWERUP;
        else if (ampl_effect == "VIBRATO")
          instr.ampl_effect = AmplitudeType::VIBRATO;
          
        if (phase_effect.empty() || phase_effect == "ZERO")
          instr.phase_effect = PhaseType::ZERO;
        
        while (iss >> modifier)
        {
          auto col_idx = modifier.find(':');
          if (col_idx != std::string::npos)
          {
            auto modifier_name = modifier.substr(0, col_idx);
            auto modifier_val = modifier.substr(col_idx + 1);
            
            if (modifier_name == "vol")
            {
              if (!(std::istringstream(modifier_val) >> vol))
                std::cerr << "Error parsing vol in instrument line: \"" << line << "\"." << std::endl;
            }
          }
        }
        instr.volume = vol;
      }
      else if (op.find("ring_mod_A:") == 0 || op.find("ring_mod_B:") == 0)
      {
        // Ring modulate.
        std::string ring_mod_A, ring_mod_B;
        while (iss >> modifier)
        {
          auto col_idx = modifier.find(':');
          if (col_idx != std::string::npos)
          {
            auto modifier_name = modifier.substr(0, col_idx);
            auto modifier_val = modifier.substr(col_idx + 1);
            
            if (modifier_name == "ring_mod_A")
            {
              if (!(std::istringstream(modifier_val) >> ring_mod_A))
                std::cerr << "Error parsing ring_mod_A in instrument line: \"" << line << "\"." << std::endl;
            }
            else if (modifier_name == "ring_mod_B")
            {
              if (!(std::istringstream(modifier_val) >> ring_mod_B))
                std::cerr << "Error parsing ring_mod_B in instrument line: \"" << line << "\"." << std::endl;
            }
            else if (modifier_name == "adsr")
            {
              if (!(std::istringstream(modifier_val) >> adsr_nr))
                std::cerr << "Error parsing adsr in instrument line: \"" << line << "\"." << std::endl;
            }
            else if (modifier_name == "flp")
            {
              if (!(std::istringstream(modifier_val) >> flp_nr))
                std::cerr << "Error parsing flp in instrument line: \"" << line << "\"." << std::endl;
            }
            else if (modifier_name == "vol")
            {
              if (!(std::istringstream(modifier_val) >> vol))
                std::cerr << "Error parsing vol in instrument line: \"" << line << "\"." << std::endl;
            }
          }
        }
        if (ring_mod_A.empty() || ring_mod_B.empty())
          std::cerr << "Must specify both attributes ring_mod_A and ring_mod_B in instrument line: \"" << line << "\"." << std::endl;
        else
          m_instruments_ring_mod.push_back({ instrument_name, ring_mod_A, ring_mod_B, adsr_nr, flp_nr, vol });
      }
      else
      {
        // Basic.
        iss >> waveform_name;
        while (iss >> modifier)
        {
          auto col_idx = modifier.find(':');
          if (col_idx != std::string::npos)
          {
            auto modifier_name = modifier.substr(0, col_idx);
            auto modifier_val = modifier.substr(col_idx + 1);
            
            if (modifier_name == "duty_cycle")
            {
              if (!(std::istringstream(modifier_val) >> duty_cycle))
                std::cerr << "Error parsing duty_cycle in instrument line: \"" << line << "\"." << std::endl;
            }
            if (modifier_name == "adsr")
            {
              if (!(std::istringstream(modifier_val) >> adsr_nr))
                std::cerr << "Error parsing adsr in instrument line: \"" << line << "\"." << std::endl;
            }
            else if (modifier_name == "flp")
            {
              if (!(std::istringstream(modifier_val) >> flp_nr))
                std::cerr << "Error parsing flp in instrument line: \"" << line << "\"." << std::endl;
            }
            else if (modifier_name == "vol")
            {
              if (!(std::istringstream(modifier_val) >> vol))
                std::cerr << "Error parsing vol in instrument line: \"" << line << "\"." << std::endl;
            }
          }
        }
        
        WaveformType wf_type = WaveformType::SINE_WAVE;
        if (waveform_name == "sine")
          wf_type = WaveformType::SINE_WAVE;
        else if (waveform_name == "square")
          wf_type = WaveformType::SQUARE_WAVE;
        else if (waveform_name == "triangle")
          wf_type = WaveformType::TRIANGLE_WAVE;
        else if (waveform_name == "sawtooth")
          wf_type = WaveformType::SAWTOOTH_WAVE;
        else if (waveform_name == "noise")
          wf_type = WaveformType::NOISE;
        else if (waveform_name == "pwm")
          wf_type = WaveformType::PWM;
        
        m_instruments_basic.push_back({ instrument_name, wf_type, duty_cycle, adsr_nr, flp_nr, vol });
      }
    }
    
    void parse_envelopes(const std::string& line, std::istringstream& iss)
    {
      // adsr <adsr_nr> <attack_mode> <attack_ms> <decay_mode> <decay_ms> <sustain_level> <release_mode> <release_ms>
      int adsr_nr = -1;
      std::string attack_mode, decay_mode, release_mode;
      float attack_ms = 0.f, decay_ms = 0.f, sustain_level = 0.f, release_ms = 0.f;
      
      iss >> adsr_nr >> attack_mode >> attack_ms >> decay_mode >> decay_ms >> sustain_level >> release_mode >> release_ms;
      if (m_envelopes.size() < adsr_nr + 1)
        m_envelopes.resize(adsr_nr + 1);
      
      auto str2mode = [](const std::string& str)
      {
        if (str == "LIN")
          return ADSRMode::LIN;
        if (str == "EXP")
          return ADSRMode::EXP;
        if (str == "LOG")
          return ADSRMode::LOG;
        return ADSRMode::LIN;
      };
      
      Attack attack { str2mode(attack_mode), attack_ms };
      Decay decay { str2mode(decay_mode), decay_ms };
      Sustain sustain { sustain_level / 100 };
      Release release { str2mode(release_mode), release_ms };
      
      m_envelopes[adsr_nr] = { attack, decay, sustain, release };
    }
    
    void parse_lp_filters(const std::string& line, std::istringstream& iss)
    {
      // filter_lp <filter_lp_nr> [type] [order] [cutoff_frq_mult] [ripple]
      
      //struct LowPassFilterArgs
      //{
      //  LowPassFilterType filter_type = LowPassFilterType::NONE;
      //  int filter_order = 1;
      //  float cutoff_freq_multiplier = 2.5f;
      //  float ripple = 0.1f;
      //};
      
      // Butterworth, ChebyshevTypeI, ChebyshevTypeII
      
      int filter_lp_nr = -1;
      std::string type;
      int order = -1;
      float cutoff = 0.f;
      float ripple = 0.f;
      
      iss >> filter_lp_nr >> type >> order >> cutoff >> ripple;
      if (m_filter_lp_args.size() < filter_lp_nr + 1)
        m_filter_lp_args.resize(filter_lp_nr + 1);
      
      auto str2type = [](const std::string& str)
      {
        if (str == "Butterworth")
          return LowPassFilterType::Butterworth;
        if (str == "ChebyshevTypeI")
          return LowPassFilterType::ChebyshevTypeI;
        if (str == "ChebyshevTypeII")
          return LowPassFilterType::ChebyshevTypeII;
        return LowPassFilterType::NONE;
      };
      
      m_filter_lp_args[filter_lp_nr] = { str2type(type), order, cutoff, ripple };
    }
    
    void parse_tab(const std::string& line, std::istringstream& iss)
    {
      std::string modifier, pitch, instrument;
      int duration_ms;
      
      auto str2pitch = [](const std::string& str) -> float
      {
        if (str.size() < 2)
          return 0;
        int aeolean_idx = str[0] - 'A';
        int mode = 0;
        if (str.size() == 3)
        {
          if (str[1] == 'b')
            mode = -1;
          else if (str[1] == '#')
            mode = +1;
        }
        int octave = str.back() - '0';
        auto f_factor = [octave](int base_octave) -> float
        {
          float f = 1.f;
          if (octave > base_octave)
            for (int o = octave; o > base_octave; --o)
              f *= 2.f;
          else
            for (int o = octave; o < base_octave; ++o)
              f *= 0.5f;
          return f;
        };
        float freq_Hz = 0.f;
        switch (aeolean_idx)
        {
          case 0: // A6 / Ab6(G#6) / A#6(Bb6)
            freq_Hz = mode == 0 ? 1760.00f : (mode == -1 ? 1661.22f : 1864.66f);
            freq_Hz *= f_factor(6);
            break;
          case 1: // B6 / Bb6(A#6) / B#6(C7)
            freq_Hz = mode == 0 ? 1975.53f : (mode == -1 ? 1864.66f : 2093.00f);
            freq_Hz *= f_factor(6);
            break;
          case 2: // C7 / Cb7(B6) / C#7(Db7)
            freq_Hz = mode == 0 ? 2093.00f : (mode == -1 ? 1975.53f : 2217.46f);
            freq_Hz *= f_factor(7);
            break;
          case 3: // D7 / Db7(C#7) / D#7(Eb7)
            freq_Hz = mode == 0 ? 2349.32f : (mode == -1 ? 2217.46f : 2489.02f);
            freq_Hz *= f_factor(7);
            break;
          case 4: // E7 / Eb7(D#7) / E#7(F7)
            freq_Hz = mode == 0 ? 2637.02f : (mode == -1 ? 2489.02f : 2793.83f);
            freq_Hz *= f_factor(7);
            break;
          case 5: // F7 / Fb7(E7) / F#7(Gb7)
            freq_Hz = mode == 0 ? 2793.83f : (mode == -1 ? 2637.02f : 2959.96f);
            freq_Hz *= f_factor(7);
            break;
          case 6: // G7 / Gb7(F#7) / G#7(Ab7)
            freq_Hz = mode == 0 ? 3135.96f : (mode == -1 ? 2959.96f : 3322.44f);
            freq_Hz *= f_factor(7);
            break;
        }
        return freq_Hz;
      };
    
      int voice_idx = 0;
      while (iss >> modifier)
      {
        if (modifier == "-")
        {
          for (auto& voice : m_voices)
            voice.notes.emplace_back(std::make_unique<Note>());
          break;
        }
        else if (modifier == "|" && voice_idx < num_voices)
        {
          auto& voice = m_voices[voice_idx++];
          
          std::string unread = iss.eof()  ?  "" : iss.str().substr(iss.tellg());
          auto op = str::ltrim_ret(unread);
          
          if (!op.empty() && op[0] == '-')
          {
            std::string delim;
            iss >> delim;
            if (delim != "-")
              std::cerr << "Error: Incorrect format. Voice note must be closed with a '|' delimiter." << std::endl;
            voice.notes.emplace_back(std::make_unique<Note>());
          }
          else if (iss >> pitch >> duration_ms >> instrument)
          {
            auto freq_Hz = str2pitch(pitch);
            auto* note = voice.notes.emplace_back(std::make_unique<Note>(freq_Hz, duration_ms)).get();
            
            auto f_match_instr = [instrument](const auto& instr) { return instr.name == instrument; };
            note->instrument_basic_idx = stlutils::find_if_idx(m_instruments_basic, f_match_instr);
            note->instrument_ring_mod_idx = stlutils::find_if_idx(m_instruments_ring_mod, f_match_instr);
            note->instrument_weight_avg_idx = stlutils::find_if_idx(m_instruments_weight_avg, f_match_instr);
            note->instrument_lib_idx = stlutils::find_if_idx(m_instruments_lib, f_match_instr);
          }
          else
          {
            std::cerr << "Error: Incorrect format in line: \"" << line << "\"." << std::endl;
          }
        }
      }
    }
    
    Waveform create_waveform(Note* note, const std::string& instr_name)
    {
      auto f_match_instr_name = [&instr_name](const auto& i) { return i.name == instr_name; };
      auto it_ib = stlutils::find_if(m_instruments_basic, f_match_instr_name);
      if (it_ib != m_instruments_basic.end())
        return create_instrument_basic(note, *it_ib);
      
      auto it_irm = stlutils::find_if(m_instruments_ring_mod, f_match_instr_name);
      if (it_irm != m_instruments_ring_mod.end())
        return create_instrument_ring_mod(note, *it_irm);
      
      auto it_iwa = stlutils::find_if(m_instruments_weight_avg, f_match_instr_name);
      if (it_iwa != m_instruments_weight_avg.end())
        return create_instrument_weight_avg(note, *it_iwa);
      
      auto it_il = stlutils::find_if(m_instruments_lib, f_match_instr_name);
      if (it_il != m_instruments_lib.end())
        return create_instrument_lib(note, *it_il);
      
      return {};
    };
    
    Waveform create_instrument_basic(Note* note, const InstrumentBasic& ib)
    {
      Waveform wave;
      wave = m_waveform_gen.generate_waveform(ib.waveform, note->duration_ms*1e-3f, note->frequency,
                                              FrequencyType::CONSTANT, AmplitudeType::CONSTANT, PhaseType::ZERO, ib.duty_cycle);
      if (ib.flp_idx >= 0)
      {
        const auto& flp = m_filter_lp_args[ib.flp_idx];
        wave = WaveformHelper::filter_low_pass(wave, flp);
      }
      if (ib.adsr_idx >= 0)
      {
        const auto& adsr = m_envelopes[ib.adsr_idx];
        wave = WaveformHelper::envelope_adsr(wave, adsr);
      }
      return wave;
    }
    
    Waveform create_instrument_ring_mod(Note* note, const InstrumentRingMod& irm)
    {
      Waveform wave;
      
      Waveform wave_A = create_waveform(note, irm.ring_mod_instr_name_A);
      Waveform wave_B = create_waveform(note, irm.ring_mod_instr_name_B);
      
      wave = WaveformHelper::ring_modulation(wave_A, wave_B);
      
      if (irm.flp_idx >= 0)
      {
        const auto& flp = m_filter_lp_args[irm.flp_idx];
        wave = WaveformHelper::filter_low_pass(wave, flp);
      }
      if (irm.adsr_idx >= 0)
      {
        const auto& adsr = m_envelopes[irm.adsr_idx];
        wave = WaveformHelper::envelope_adsr(wave, adsr);
      }
      
      return wave;
    }
    
    Waveform create_instrument_weight_avg(Note* note, const InstrumentWeightAvg& iwa)
    {
      Waveform wave;
      
      std::vector<std::pair<float, Waveform>> weighted_waves;
      for (const auto& iwp : iwa.instrument_names)
      {
        auto ww = create_waveform(note, iwp.second);
        weighted_waves.emplace_back(iwp.first, ww);
      }
      
      wave = WaveformHelper::mix(weighted_waves);
      
      if (iwa.flp_idx >= 0)
      {
        const auto& flp = m_filter_lp_args[iwa.flp_idx];
        wave = WaveformHelper::filter_low_pass(wave, flp);
      }
      if (iwa.adsr_idx >= 0)
      {
        const auto& adsr = m_envelopes[iwa.adsr_idx];
        wave = WaveformHelper::envelope_adsr(wave, adsr);
      }
      
      return wave;
    }
    
    Waveform create_instrument_lib(Note* note, const InstrumentLib& il)
    {
      Waveform wave;
      
      wave = Synthesizer::synthesize(il.lib_instrument, m_waveform_gen,
        note->duration_ms * 1e-3f, note->frequency,
        il.freq_effect, il.ampl_effect, il.phase_effect);
      
      return wave;
    }
    
    void create_instruments()
    {
      for (auto& voice : m_voices)
      {
        for (auto& note : voice.notes)
        {
          if (note->pause)
          {}
          else if (note->instrument_basic_idx >= 0)
          {
            const auto& ib = m_instruments_basic[note->instrument_basic_idx];
            note->wave = create_instrument_basic(note.get(), ib);
            note->volume = ib.volume;
          }
          else if (note->instrument_ring_mod_idx >= 0)
          {
            const auto& irm = m_instruments_ring_mod[note->instrument_ring_mod_idx];
            note->wave = create_instrument_ring_mod(note.get(), irm);
            note->volume = irm.volume;
          }
          else if (note->instrument_weight_avg_idx >= 0)
          {
            const auto& iwa = m_instruments_weight_avg[note->instrument_weight_avg_idx];
            note->wave = create_instrument_weight_avg(note.get(), iwa);
            note->volume = iwa.volume;
          }
          else if (note->instrument_lib_idx >= 0)
          {
            const auto& it = m_instruments_lib[note->instrument_lib_idx];
            note->wave = create_instrument_lib(note.get(), it);
            note->volume = it.volume;
          }
        }
      }
    }
    
    void init_voice_sources()
    {
      for (auto& voice : m_voices)
      {
        // #FIXME: Use streamed audio source
        voice.src = m_audio_handler.create_stream_source();
      }
    }
    
    std::thread audio_thread;
    std::atomic<bool> stop_audio_thread { false };
  };

}
