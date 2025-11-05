//
//  ChipTuneEngineParser.h
//  8Beat
//
//  Created by Rasmus Anthin on 2024-03-16.
//

#pragma once

#include "../Synthesizer.h"
#include <Core/Utils.h>

#include <iostream>
#include <fstream>
#include <sstream>


namespace beat
{

  class ChipTuneEngineParser
  {
    void clear()
    {
      m_instruments_basic.clear();
      m_instruments_ring_mod.clear();
      m_instruments_conv.clear();
      m_instruments_weight_avg.clear();
      m_instruments_lib.clear();
      m_envelopes.clear();
      m_filter_args.clear();
      m_waveform_params.clear();
      
          
      m_time_step_ms.clear();
      m_curr_time_step_ms = 100;
    
      m_gain.clear();
      m_curr_gain = 1.f;
    
      m_labels.clear(); // note_idx -> Label (label, id)
      m_gotos.clear(); // note_idx -> Goto (from_label, to_label, count)
      m_al_fine = false;
      m_al_coda = false;
      m_to_coda = false;
    
      m_print_switches.clear();
    
      num_voices = 0;
      m_voices.clear();
      //std::vector<Instrument> m_instruments;
      note_start_idx = 0;
      num_notes_parsed = 0;
    }
    
  public:
    ChipTuneEngineParser(AudioSourceHandler& audio_handler, const WaveformGeneration& waveform_gen)
      : m_audio_handler(audio_handler)
      , m_waveform_gen(waveform_gen)
    {
    }
    virtual ~ChipTuneEngineParser()
    {
      for (const auto& voice : m_voices)
        m_audio_handler.remove_source(voice.src);
    }
  
    // Load tune from a text file with a specific format
    bool load_tune(const std::string& file_path, bool verbose = false)
    {
      clear();
    
      if (!file_path.ends_with(".ct"))
      {
        std::cerr << "Wrong file ending in filepath argument. Expected *.ct" << std::endl;
        return false;
      }
    
      std::ifstream file(file_path);
      if (!file.is_open())
      {
        std::cerr << "Error opening tune file: " << file_path << std::endl;
        return false;
      }
      
      m_curr_file_path = file_path;

      if (verbose)
        std::cout << "Parsing Tune" << std::endl;
      std::string line;
      while (std::getline(file, line))
      {
        if (!parse_line(line))
          break;
      }
      for (auto& voice : m_voices)
        voice.notes.emplace_back(std::make_unique<Note>(Note::create_separator()));
      
      if (verbose)
        std::cout << "Creating Instruments" << std::endl;
      create_instruments();
      if (verbose)
        std::cout << "Initializing Sources" << std::endl;
      init_voice_sources();

      return true;
    }
    
  protected:
    struct Note
    {
      Note() = default;
      Note(float f, float d) : pause(false), frequency(f), duration_ms(d) {}
      bool pause = false;
      bool separator = false; // Separates commands
      float frequency = 0.f;
      float duration_ms = 0.f;
      Waveform wave;
      int instrument_basic_idx = -1;
      int instrument_ring_mod_idx = -1;
      int instrument_conv_idx = -1;
      int instrument_weight_avg_idx = -1;
      int instrument_lib_idx = -1;
      int adsr_idx = -1;
      int flt_idx = -1;
      float gain = 1.f;
      static Note create_pause()
      {
        Note n;
        n.pause = true;
        return n;
      }
      static Note create_separator()
      {
        Note n;
        n.separator = true;
        return n;
      }
      static Note create_note(float f, float d)
      {
        Note n(f, d);
        return n;
      }
    };
    struct InstrumentBase
    {
      std::string name;
      int adsr_idx = -1;
      int flt_idx = -1;
      float gain = 1.f;
    };
    struct InstrumentBasic : InstrumentBase
    {
      WaveformType waveform = WaveformType::SINE;
      int params_idx = -1;
      FrequencyType freq_effect = FrequencyType::CONSTANT;
      AmplitudeType ampl_effect = AmplitudeType::CONSTANT;
      PhaseType phase_effect = PhaseType::ZERO;
    };
    struct InstrumentRingMod : InstrumentBase
    {
      std::string ring_mod_instr_name_A;
      std::string ring_mod_instr_name_B;
    };
    struct InstrumentConv : InstrumentBase
    {
      std::string conv_instr_name_A;
      std::string conv_instr_name_B;
    };
    struct InstrumentWeightAvg : InstrumentBase
    {
      std::vector<std::pair<float, std::string>> instrument_names;
    };
    struct InstrumentLib : InstrumentBase
    {
      InstrumentType lib_instrument = InstrumentType::PIANO;
      FrequencyType freq_effect = FrequencyType::CONSTANT;
      AmplitudeType ampl_effect = AmplitudeType::CONSTANT;
      PhaseType phase_effect = PhaseType::ZERO;
    };
    struct Voice
    {
      AudioSource* src = nullptr;
      std::vector<std::unique_ptr<Note>> notes;
    };
    class Goto
    {
      int orig_count = 0;
      
    public:
      Goto() = default;
      Goto(const std::string& src_lbl, const std::string& dst_lbl, int cnt, int curr_idx)
        : orig_count(cnt)
        , from_label(src_lbl)
        , to_label(dst_lbl)
        , count(cnt)
        , note_idx(curr_idx)
      {}
      
      std::string from_label, to_label;
      int count = 0;
      int note_idx = -1; // Position of the goto itself.
      
      void reset() { count = orig_count; }
      // Number of jumps performed so far.
      int num_jumps() const { return orig_count - count; }
    };
    struct Label
    {
      Label(const std::string& a_lbl, int a_id, Goto* a_src_goto = nullptr)
        : label(a_lbl)
        , id(a_id)
        , src_goto(a_src_goto)
      {}
      std::string label;
      // For ENDING command:
      int id = 0;
      Goto* src_goto = nullptr; // src_goto before its jump to its label.
      std::vector<std::pair<int, Label*>> related_labels;
    };

    AudioSourceHandler& m_audio_handler;
    const WaveformGeneration& m_waveform_gen;
    std::string m_curr_file_path;
    std::vector<InstrumentBasic> m_instruments_basic;
    std::vector<InstrumentRingMod> m_instruments_ring_mod;
    std::vector<InstrumentConv> m_instruments_conv;
    std::vector<InstrumentWeightAvg> m_instruments_weight_avg;
    std::vector<InstrumentLib> m_instruments_lib;
    std::vector<ADSR> m_envelopes;
    std::vector<FilterArgs> m_filter_args;
    std::vector<WaveformGenerationParams> m_waveform_params;
    
    std::map<int, float> m_time_step_ms;
    float m_curr_time_step_ms = 100;
    
    std::map<int, float> m_gain;
    float m_curr_gain = 1.f;
    
    std::map<int, std::unique_ptr<Label>> m_labels; // note_idx -> Label (label, id)
    std::map<int, std::unique_ptr<Goto>> m_gotos; // note_idx -> Goto (from_label, to_label, count)
    bool m_al_fine = false;
    bool m_al_coda = false;
    bool m_to_coda = false;
    
    std::map<int, bool> m_print_switches;
    
    int num_voices = 0;
    std::vector<Voice> m_voices;
    //std::vector<Instrument> m_instruments;
    int note_start_idx = 0;
    int num_notes_parsed = 0;
    

    bool parse_line(const std::string& line)
    {
      auto f_find_label = [this](const auto& label)
      {
        return std::find_if(m_labels.begin(), m_labels.end(), [&label](const auto& lp) { return lp.second->label == label; });
      };
      
      auto f_has_goto_from_label = [this](const auto& from_label)
      {
        return std::find_if(m_gotos.begin(), m_gotos.end(), [&from_label](const auto& gp) { return gp.second->from_label == from_label; }) != m_gotos.end();
      };
      
      m_gain[0] = 1.f;
    
      std::istringstream iss(line);
      if (!line.empty())
      {
        std::string command;
        if (iss >> command)
        {
          auto num_gotos_prev = m_gotos.size();
          
          if (command == "instrument")
            parse_instrument(line, iss);
          else if (command == "adsr")
            parse_envelopes(line, iss);
          else if (command == "filter")
            parse_filters(line, iss);
          else if (command == "params")
            parse_waveform_params(line, iss);
          else if (command == "NUM_VOICES")
          {
            iss >> num_voices;
            if (num_voices > 0)
              m_voices.resize(num_voices);
          }
          else if (command == "TIME_STEP_MS")
          {
            iss >> m_curr_time_step_ms;
            m_time_step_ms[num_notes_parsed] = m_curr_time_step_ms;
          }
          else if (command == "GAIN")
          {
            iss >> m_curr_gain;
            m_gain[num_notes_parsed] = m_curr_gain;
          }
          else if (command == "LABEL")
          {
            std::string label;
            iss >> label;
            m_labels[num_notes_parsed] = std::make_unique<Label>(label, 0);
          }
          else if (command == "GOTO")
          {
            std::string goto_lbl;
            iss >> goto_lbl;
            m_gotos[num_notes_parsed] = std::make_unique<Goto>("GOTO", goto_lbl, -1, num_notes_parsed);
          }
          else if (command == "GOTO_TIMES")
          {
            std::string goto_lbl;
            int count = 0;
            iss >> goto_lbl >> count;
            m_gotos[num_notes_parsed] = std::make_unique<Goto>("GOTO_TIMES", goto_lbl, count, num_notes_parsed);
            int start_idx = 0;
            int end_idx = num_notes_parsed;
            if (auto it = f_find_label(goto_lbl); it != m_labels.end())
              start_idx = it->first;
            std::vector<std::pair<int, Label*>> ending_labels;
            for (auto it = m_labels.begin(); it != m_labels.end(); ++it)
              if (start_idx <= it->first && it->first <= end_idx)
                if (it->second->label == "ENDING")
                {
                  it->second->src_goto = m_gotos[num_notes_parsed].get();
                  ending_labels.emplace_back(it->first, it->second.get());
                }
            for (auto& lbl_pair : ending_labels)
              lbl_pair.second->related_labels = ending_labels;
          }
          else if (command == "CODA")
          {
            if (f_find_label("CODA") == m_labels.end())
              m_labels[num_notes_parsed] = std::make_unique<Label>("CODA", -2);
            else
              std::cerr << "CODA already defined previously!" << std::endl;
          }
          else if (command == "SEGNO")
          {
            if (f_find_label("SEGNO") == m_labels.end())
              m_labels[num_notes_parsed] = std::make_unique<Label>("SEGNO", -2);
            else
              std::cerr << "SEGNO already defined previously!" << std::endl;
          }
          else if (command == "FINE")
          {
            if (f_find_label("FINE") == m_labels.end())
              m_labels[num_notes_parsed] = std::make_unique<Label>("FINE", -2);
            else
              std::cerr << "FINE already defined previously!" << std::endl;
          }
          else if (command == "DA_CAPO_AL_FINE")
          {
            if (!f_has_goto_from_label("DA_CAPO_AL_FINE"))
              m_gotos[num_notes_parsed] = std::make_unique<Goto>("DA_CAPO_AL_FINE", "", -2, num_notes_parsed);
            else
              std::cerr << "DA_CAPO_AL_FINE already defined previously!" << std::endl;
          }
          else if (command == "DA_CAPO_AL_CODA")
          {
            if (!f_has_goto_from_label("DA_CAPO_AL_CODA"))
              m_gotos[num_notes_parsed] = std::make_unique<Goto>("DA_CAPO_AL_CODA", "", -2, num_notes_parsed);
            else
              std::cerr << "DA_CAPO_AL_CODA already defined previously!" << std::endl;
          }
          else if (command == "DAL_SEGNO_AL_FINE")
          {
            if (!f_has_goto_from_label("DAL_SEGNO_AL_FINE"))
              m_gotos[num_notes_parsed] = std::make_unique<Goto>("DAL_SEGNO_AL_FINE", "", -2, num_notes_parsed);
            else
              std::cerr << "DAL_SEGNO_AL_FINE already defined previously!" << std::endl;
          }
          else if (command == "DAL_SEGNO_AL_CODA")
          {
            if (!f_has_goto_from_label("DAL_SEGNO_AL_CODA"))
              m_gotos[num_notes_parsed] = std::make_unique<Goto>("DAL_SEGNO_AL_CODA", "", -2, num_notes_parsed);
            else
              std::cerr << "DAL_SEGNO_AL_CODA already defined previously!" << std::endl;
          }
          else if (command == "TO_CODA")
          {
            if (!f_has_goto_from_label("TO_CODA"))
              m_gotos[num_notes_parsed] = std::make_unique<Goto>("TO_CODA", "CODA", -2, num_notes_parsed);
            else
              std::cerr << "TO_CODA already defined previously!" << std::endl;
          }
          else if (command == "ENDING")
          {
            int count = 0;
            iss >> count;
            m_labels[num_notes_parsed] = std::make_unique<Label>("ENDING", count);
          }
          else if (command == "PRINT")
          {
            std::string on_off;
            iss >> on_off;
            if (on_off == "ON")
              m_print_switches[num_notes_parsed] = true;
            else if (on_off == "OFF")
              m_print_switches[num_notes_parsed] = false;
          }
          else if (command == "TAB")
          {
            parse_tab(line, iss);
            num_notes_parsed++;
          }
          else if (command == "END")
            return false;
          else if (command == "START")
            note_start_idx = num_notes_parsed; // + stlutils::contains_if(m_gotos, [this](const auto& gp) { return gp.first == num_notes_parsed; }); // #HACK! Is there a better way?
          
          if (m_gotos.size() > num_gotos_prev)
          {
            for (int v_idx = 0; v_idx < num_voices; ++v_idx)
              m_voices[v_idx].notes.emplace_back(std::make_unique<Note>(Note::create_separator()));
            num_notes_parsed++;
          }
        }
        else
          std::cerr << "Error parsing instrument line: " << line << std::endl;
      }
      return true;
    }
    
    bool parse_post_effects(const std::string& line,
                            const std::string& modifier_name, const std::string& modifier_val,
                            int& adsr_nr, int& flt_nr, float& gain)
    {
      if (modifier_name == "adsr")
      {
        if (!(std::istringstream(modifier_val) >> adsr_nr))
          std::cerr << "Error parsing adsr in instrument line: \"" << line << "\"." << std::endl;
        return true;
      }
      if (modifier_name == "flt")
      {
        if (!(std::istringstream(modifier_val) >> flt_nr))
          std::cerr << "Error parsing flt in instrument line: \"" << line << "\"." << std::endl;
        return true;
      }
      if (modifier_name == "gain")
      {
        if (!(std::istringstream(modifier_val) >> gain))
          std::cerr << "Error parsing gain in instrument line: \"" << line << "\"." << std::endl;
        return true;
      }
      return false;
    }
    
    bool parse_waveform_effects(const std::string& line,
                                const std::string& modifier_name, const std::string& modifier_val,
                                int& params_nr)
    {
      if (modifier_name == "params")
      {
        if (!(std::istringstream(modifier_val) >> params_nr))
          std::cerr << "Error parsing params in instrument line: \"" << line << "\"." << std::endl;
        return true;
      }
      return false;
    }
    
    bool parse_modulation_effects(const std::string& line,
      const std::string& modifier_name, const std::string& modifier_val,
      FrequencyType& frequency_effect, AmplitudeType& amplitude_effect, PhaseType& phase_effect)
    {
      std::string str_freq_effect, str_ampl_effect, str_phase_effect;
      
      if (modifier_name == "ffx")
      {
        if (!(std::istringstream(modifier_val) >> str_freq_effect))
          std::cerr << "Error parsing ffx in instrument line: \"" << line << "\"." << std::endl;
        if (str_freq_effect.empty() || str_freq_effect == "CONSTANT")
          frequency_effect = FrequencyType::CONSTANT;
        else if (str_freq_effect == "JET_ENGINE_POWERUP")
          frequency_effect = FrequencyType::JET_ENGINE_POWERUP;
        else if (str_freq_effect == "CHIRP_0")
          frequency_effect = FrequencyType::CHIRP_0;
        else if (str_freq_effect == "CHIRP_1")
          frequency_effect = FrequencyType::CHIRP_1;
        else if (str_freq_effect == "CHIRP_2")
          frequency_effect = FrequencyType::CHIRP_2;
        return true;
      }
      if (modifier_name == "afx")
      {
        if (!(std::istringstream(modifier_val) >> str_ampl_effect))
          std::cerr << "Error parsing afx in instrument line: \"" << line << "\"." << std::endl;
        if (str_ampl_effect.empty() || str_ampl_effect == "CONSTANT")
          amplitude_effect = AmplitudeType::CONSTANT;
        else if (str_ampl_effect == "JET_ENGINE_POWERUP")
          amplitude_effect = AmplitudeType::JET_ENGINE_POWERUP;
        else if (str_ampl_effect == "VIBRATO_0")
          amplitude_effect = AmplitudeType::VIBRATO_0;
        return true;
      }
      if (modifier_name == "pfx")
      {
        if (!(std::istringstream(modifier_val) >> str_phase_effect))
          std::cerr << "Error parsing pfx in instrument line: \"" << line << "\"." << std::endl;
        if (str_phase_effect.empty() || str_phase_effect == "ZERO")
          phase_effect = PhaseType::ZERO;
        return true;
      }
      return false;
    }
    
    void parse_instrument(const std::string& line, std::istringstream& iss)
    {
      std::string instrument_name, waveform_name, modifier;
      std::string op;
      float gain = 1.f;
      int params_nr = -1, adsr_nr = -1, flt_nr = -1;
      
      iss >> instrument_name;

      std::string unread = iss.eof()  ?  "" : iss.str().substr(iss.tellg());
      op = str::ltrim_ret(unread);
      
      if (op.find("(") == 0)
      {
        // Weighted average.
        auto idx = op.find("adsr");
        math::minimize(idx, op.find("flt"));
        math::minimize(idx, op.find("gain"));
        idx = idx != std::string::npos ? idx - 1 : std::string::npos;
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
            
            parse_post_effects(line, modifier_name, modifier_val,
              adsr_nr, flt_nr, gain);
          }
        }
        instrument.adsr_idx = adsr_nr;
        instrument.flt_idx = flt_nr;
        instrument.gain = gain;
      }
      else if (op.find("&") == 0)
      {
        std::string lib_instrument;
        // Library instrument.
        iss >> lib_instrument;
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
        else if (lib_instrument == "GUITAR")
          instr.lib_instrument = InstrumentType::GUITAR;
        else if (lib_instrument == "KICKDRUM")
          instr.lib_instrument = InstrumentType::KICKDRUM;
        else if (lib_instrument == "SNAREDRUM")
          instr.lib_instrument = InstrumentType::SNAREDRUM;
        else if (lib_instrument == "HIHAT")
          instr.lib_instrument = InstrumentType::HIHAT;
        else if (lib_instrument == "ANVIL")
          instr.lib_instrument = InstrumentType::ANVIL;
        
        while (iss >> modifier)
        {
          auto col_idx = modifier.find(':');
          if (col_idx != std::string::npos)
          {
            auto modifier_name = modifier.substr(0, col_idx);
            auto modifier_val = modifier.substr(col_idx + 1);
            
            if (parse_modulation_effects(line, modifier_name, modifier_val,
              instr.freq_effect, instr.ampl_effect, instr.phase_effect))
            {}
            else if (parse_post_effects(line, modifier_name, modifier_val,
              adsr_nr, flt_nr, gain))
            {}
          }
        }
        instr.adsr_idx = adsr_nr;
        instr.flt_idx = flt_nr;
        instr.gain = gain;
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
            else
              parse_post_effects(line, modifier_name, modifier_val,
                adsr_nr, flt_nr, gain);
          }
        }
        if (ring_mod_A.empty() || ring_mod_B.empty())
          std::cerr << "Must specify both attributes ring_mod_A and ring_mod_B in instrument line: \"" << line << "\"." << std::endl;
        else
          m_instruments_ring_mod.push_back({ { instrument_name, adsr_nr, flt_nr, gain }, ring_mod_A, ring_mod_B });
      }
      else if (op.find("conv_A:") == 0 || op.find("conv_B:") == 0)
      {
        // Convolution.
        std::string conv_A, conv_B;
        while (iss >> modifier)
        {
          auto col_idx = modifier.find(':');
          if (col_idx != std::string::npos)
          {
            auto modifier_name = modifier.substr(0, col_idx);
            auto modifier_val = modifier.substr(col_idx + 1);
            
            if (modifier_name == "conv_A")
            {
              if (!(std::istringstream(modifier_val) >> conv_A))
                std::cerr << "Error parsing conv_A in instrument line: \"" << line << "\"." << std::endl;
            }
            else if (modifier_name == "conv_B")
            {
              if (!(std::istringstream(modifier_val) >> conv_B))
                std::cerr << "Error parsing conv_B in instrument line: \"" << line << "\"." << std::endl;
            }
            else
              parse_post_effects(line, modifier_name, modifier_val,
                adsr_nr, flt_nr, gain);
          }
        }
        if (conv_A.empty() || conv_B.empty())
          std::cerr << "Must specify both attributes ring_mod_A and ring_mod_B in instrument line: \"" << line << "\"." << std::endl;
        else
          m_instruments_conv.push_back({ { instrument_name, adsr_nr, flt_nr, gain }, conv_A, conv_B });
      }
      else
      {
        // Basic.
        FrequencyType freq_effect = FrequencyType::CONSTANT;
        AmplitudeType ampl_effect = AmplitudeType::CONSTANT;
        PhaseType phase_effect = PhaseType::ZERO;
        
        iss >> waveform_name;
        while (iss >> modifier)
        {
          auto col_idx = modifier.find(':');
          if (col_idx != std::string::npos)
          {
            auto modifier_name = modifier.substr(0, col_idx);
            auto modifier_val = modifier.substr(col_idx + 1);
            
            if (parse_waveform_effects(line, modifier_name, modifier_val,
              params_nr))
            {}
            else if (parse_modulation_effects(line, modifier_name, modifier_val,
              freq_effect, ampl_effect, phase_effect))
            {}
            else if (parse_post_effects(line, modifier_name, modifier_val,
              adsr_nr, flt_nr, gain))
            {}
          }
        }
        
        WaveformType wf_type = WaveformType::SINE;
        auto waveform_name_upper = str::to_upper(waveform_name);
        if (waveform_name_upper == "SINE")
          wf_type = WaveformType::SINE;
        else if (waveform_name_upper == "SQUARE")
          wf_type = WaveformType::SQUARE;
        else if (waveform_name_upper == "TRIANGLE")
          wf_type = WaveformType::TRIANGLE;
        else if (waveform_name_upper == "SAWTOOTH")
          wf_type = WaveformType::SAWTOOTH;
        else if (waveform_name_upper == "NOISE")
          wf_type = WaveformType::NOISE;
        
        m_instruments_basic.push_back({ { instrument_name, adsr_nr, flt_nr, gain }, wf_type, params_nr,
          freq_effect, ampl_effect, phase_effect });
      }
    }
    
    void parse_envelopes(const std::string& line, std::istringstream& iss)
    {
      // adsr <adsr_nr> "["<attack_mode> <attack_ms> [<level_begin>] [<level_end>]"]"
      //                "["<decay_mode> <decay_ms> [<level_begin>] [<level_end>]"]"
      //                "["<sustain_level> [<sustain_max_ms>]"]"
      //                "["<release_mode> <release_ms> [<level_begin>] [<level_end>]"]"
      int adsr_nr = -1;
      float sustain_level = 0.f;
      
      iss >> adsr_nr;
      
      if (static_cast<int>(m_envelopes.size()) < adsr_nr + 1)
        m_envelopes.resize(adsr_nr + 1);
      
      std::string unread = iss.eof()  ?  "" : iss.str().substr(iss.tellg());
      std::string op = str::ltrim_ret(unread);
      
      if (!op.empty() && op[0] == '&')
      {
        std::string adsr_lib;
        iss >> adsr_lib;
        
        adsr_lib.erase(0, 1);
        
        m_envelopes[adsr_nr] = adsr_presets::getter.get(adsr_lib);
      }
      else
      {
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
        
        ADSRMode mode = ADSRMode::LIN;
        float duration_ms = 0.f;
        std::optional<float> max_duration_ms = 0.f;
        std::optional<float> level_0 = std::nullopt;
        std::optional<float> level_1 = std::nullopt;
        
        auto parse_adr = [&iss, &str2mode](ADSRMode& mode, float& dur_ms, std::optional<float>& level_0, std::optional<float>& level_1)
        {
          char bracket;
          std::string mode_str;
          float lvl_0 = 0.f;
          float lvl_1 = 0.f;
          
          iss >> bracket; // Consume begin bracket.
          iss >> mode_str >> dur_ms;
          mode = str2mode(mode_str);
          
          std::string unread = iss.eof()  ?  "" : iss.str().substr(iss.tellg());
          std::string op = str::ltrim_ret(unread);
          
          if (op[0] != ']')
          {
            iss >> lvl_0;
            level_0 = lvl_0 / 100;
          }
          else
            level_0 = std::nullopt;
            
          unread = iss.eof()  ?  "" : iss.str().substr(iss.tellg());
          op = str::ltrim_ret(unread);
          if (op[0] != ']')
          {
            iss >> lvl_1;
            level_1 = lvl_1 / 100;
          }
          else
            level_1 = std::nullopt;
            
          iss >> bracket; // Consume end bracket.
        };
        auto parse_s = [&iss](float& sus_lvl, std::optional<float>& max_dur_ms)
        {
          char bracket;
          float t_max = 0.f;
          
          iss >> bracket; // Consume begin bracket.
          iss >> sus_lvl;
          
          std::string unread = iss.eof()  ?  "" : iss.str().substr(iss.tellg());
          std::string op = str::ltrim_ret(unread);
          
          if (op[0] != ']')
          {
            iss >> t_max;
            max_dur_ms = t_max;
          }
          else
            max_dur_ms = std::nullopt;
            
          iss >> bracket; // Consume end bracket.
        };
        //[LIN 100 0 50] [EXP 300] [50] [LOG 500]
        //[EXP 80 0] [LOG 300 80] [50 20] [EXP 500 55] // max sustain time = 20 ms.
        parse_adr(mode, duration_ms, level_0, level_1);
        Attack attack { mode, duration_ms, level_0, level_1 };
        
        parse_adr(mode, duration_ms, level_0, level_1);
        Decay decay { mode, duration_ms, level_0, level_1 };
        
        parse_s(sustain_level, max_duration_ms);
        Sustain sustain { sustain_level / 100, max_duration_ms };
        
        parse_adr(mode, duration_ms, level_0, level_1);
        Release release { mode, duration_ms, };
        
        m_envelopes[adsr_nr] = { attack, decay, sustain, release };
      }
    }
    
    void parse_filters(const std::string& line, std::istringstream& iss)
    {
      // filter <filter_nr> [type] [op_type] [order] [cutoff_frq_mult] [bandwidth_frq_mult] [ripple] [normalize]
      
      // Butterworth, ChebyshevTypeI, ChebyshevTypeII
      
      int filter_nr = -1;
      std::string type, op_type;
      int order = -1;
      float cutoff = 0.f;
      float bandwidth = 0.f;
      float ripple = 0.f;
      bool normalize = false;
      
      iss >> filter_nr >> type >> op_type >> order >> cutoff >> bandwidth >> ripple >> normalize;
      if (static_cast<int>(m_filter_args.size()) < filter_nr + 1)
        m_filter_args.resize(filter_nr + 1);
      
      auto str2type = [](const std::string& str)
      {
        if (str == "Butterworth")
          return FilterType::Butterworth;
        if (str == "ChebyshevTypeI")
          return FilterType::ChebyshevTypeI;
        if (str == "ChebyshevTypeII")
          return FilterType::ChebyshevTypeII;
        return FilterType::NONE;
      };
      
      auto str2optype = [](const std::string& str)
      {
        if (str == "LowPass")
          return FilterOpType::LowPass;
        if (str == "HighPass")
          return FilterOpType::HighPass;
        if (str == "BandPass")
          return FilterOpType::BandPass;
        if (str == "BandStop")
          return FilterOpType::BandStop;
        return FilterOpType::NONE;
      };
      
      std::optional<float> bandwidth_val;
      if (bandwidth > 0.f)
        bandwidth_val = bandwidth;
      m_filter_args[filter_nr] =
      {
        str2type(type),
        str2optype(op_type),
        order,
        cutoff,
        bandwidth_val,
        ripple,
        normalize
      };
    }
    
    void parse_waveform_params(const std::string& line, std::istringstream& iss)
    {
      int params_nr = -1;
      
      iss >> params_nr;
      if (static_cast<int>(m_waveform_params.size()) < params_nr + 1)
        m_waveform_params.resize(params_nr + 1);
      
      std::string modifier, modifier_name, modifier_val;
      WaveformGenerationParams params;
      while (iss >> modifier)
      {
        auto col_idx = modifier.find(':');
        if (col_idx != std::string::npos)
        {
          auto modifier_name = modifier.substr(0, col_idx);
          auto modifier_val = modifier.substr(col_idx + 1);
          
          auto f_parse_ofloat_val = [&modifier_name, &modifier_val, &line]
                                    (const std::string& name, std::optional<float>& set_val)
          {
            if (modifier_name == name)
            {
              float fval = 0.f;
              if (!(std::istringstream(modifier_val) >> fval))
                std::cerr << "Error parsing " << name << " in params line: \"" << line << "\"." << std::endl;
              else
                set_val = fval;
              return true;
            }
            return false;
          };
          auto f_parse_val = [&modifier_name, &modifier_val, &line]
                             (const std::string& name, auto& set_val)
          {
            if (modifier_name == name)
            {
              if (!(std::istringstream(modifier_val) >> set_val))
                std::cerr << "Error parsing " << name << " in params line: \"" << line << "\"." << std::endl;
              return true;
            }
            return false;
          };
          
          if (f_parse_ofloat_val("sample_min", params.sample_range_min)) {}
          else if (f_parse_ofloat_val("sample_max", params.sample_range_max)) {}
          else if (f_parse_ofloat_val("duty_cycle", params.duty_cycle)) {}
          else if (f_parse_ofloat_val("duty_cycle_sweep", params.duty_cycle_sweep)) {}
          else if (f_parse_ofloat_val("min_freq_limit", params.min_frequency_limit)) {}
          else if (f_parse_ofloat_val("max_freq_limit", params.max_frequency_limit)) {}
          else if (f_parse_ofloat_val("freq_slide_vel", params.freq_slide_vel)) {}
          else if (f_parse_ofloat_val("freq_slide_acc", params.freq_slide_acc)) {}
          else if (f_parse_ofloat_val("vib_depth", params.vibrato_depth)) {}
          else if (f_parse_ofloat_val("vib_freq", params.vibrato_freq)) {}
          else if (f_parse_ofloat_val("vib_freq_vel", params.vibrato_freq_vel)) {}
          else if (f_parse_ofloat_val("vib_freq_acc", params.vibrato_freq_acc)) {}
          else if (f_parse_ofloat_val("vib_freq_acc_max_vel_lim", params.vibrato_freq_acc_max_vel_limit)) {}
          else if (f_parse_val("noise_flt_order", params.noise_filter_order)) {}
          else if (f_parse_val("noise_flt_rel_bw", params.noise_filter_rel_bw)) {}
          else if (f_parse_val("noise_flt_slot_dur_s", params.noise_filter_slot_dur_s)) {}
          else if (modifier_name == "arpeggio")
          {
            const std::string op = "arpeggio:";
            auto idx = line.find(op);
            if (idx == std::string::npos)
              std::cerr << "Error parsing arpeggio operand." << std::endl;
            else
            {
              idx += op.size();
              int num_right_parentheses_in_succession = 0;
              auto idx0 = idx;
              ArpeggioPair ap { 0.f, 0.f };
              int ap_idx = 0;
              do
              {
                char ch = line[idx];
                
                if (ch == '(' || ch == ' ')
                  idx0 = idx + 1;
                else if (ch == ',' || ch == ')')
                {
                  std::string item = str::trim_ret(line.substr(idx0, idx - idx0));
                  if (item.empty())
                    params.arpeggio.emplace_back(ap);
                  else if (ap_idx == 0)
                  {
                    std::sscanf(item.c_str(), "%f", &ap.time);
                    ap_idx = 1;
                  }
                  else if (ap_idx == 1)
                  {
                    std::sscanf(item.c_str(), "%f", &ap.freq_mult);
                    ap_idx = 0;
                  }
                  idx0 = idx + 1;
                }
                  
                if (ch == ')')
                  num_right_parentheses_in_succession++;
                else
                  num_right_parentheses_in_succession = 0;
                  
                idx++;
              } while (num_right_parentheses_in_succession < 2 && idx < line.size());
              
            }
          }
        }
      }
      
      m_waveform_params[params_nr] = params;
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
            voice.notes.emplace_back(std::make_unique<Note>(Note::create_pause()));
          voice_idx = num_voices;
          break;
        }
        else if (modifier == "|" && voice_idx < num_voices && !iss.eof())
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
            voice.notes.emplace_back(std::make_unique<Note>(Note::create_pause()));
          }
          else if (iss >> pitch >> duration_ms >> instrument)
          {
            auto freq_Hz = str2pitch(pitch);
            auto* note = voice.notes.emplace_back(std::make_unique<Note>(Note::create_note(freq_Hz, static_cast<float>(duration_ms)))).get();
            
            auto f_match_instr = [instrument](const auto& instr) { return instr.name == instrument; };
            note->instrument_basic_idx = stlutils::find_if_idx(m_instruments_basic, f_match_instr);
            note->instrument_ring_mod_idx = stlutils::find_if_idx(m_instruments_ring_mod, f_match_instr);
            note->instrument_conv_idx = stlutils::find_if_idx(m_instruments_conv, f_match_instr);
            note->instrument_weight_avg_idx = stlutils::find_if_idx(m_instruments_weight_avg, f_match_instr);
            note->instrument_lib_idx = stlutils::find_if_idx(m_instruments_lib, f_match_instr);
            
            unread = iss.eof()  ?  "" : iss.str().substr(iss.tellg());
            op = str::ltrim_ret(unread);
            
            if (op.find("adsr:") == 0 || op.find("flt:") == 0 || op.find("gain:") == 0)
            {
              int adsr_nr = -1, flt_nr = -1;
              float gain = 1.f;
              while (iss >> modifier)
              {
                auto col_idx = modifier.find(':');
                if (col_idx != std::string::npos)
                {
                  auto modifier_name = modifier.substr(0, col_idx);
                  auto modifier_val = modifier.substr(col_idx + 1);
                  
                  if (parse_post_effects(line, modifier_name, modifier_val,
                                         adsr_nr, flt_nr, gain))
                  {
                    note->adsr_idx = adsr_nr;
                    note->flt_idx = flt_nr;
                    note->gain = gain;
                  }
                }
              }
            }
          }
          else
          {
            std::cerr << "Error: Incorrect format in line: \"" << line << "\"." << std::endl;
          }
        }
      }
      
      for (int v_idx = voice_idx; v_idx < num_voices; ++v_idx)
        m_voices[v_idx].notes.emplace_back(std::make_unique<Note>(Note::create_separator()));
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
        
      auto it_ic = stlutils::find_if(m_instruments_conv, f_match_instr_name);
      if (it_ic != m_instruments_conv.end())
        return create_instrument_conv(note, *it_ic);
      
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
      WaveformGenerationParams params;
      if (ib.params_idx >= 0)
        params = m_waveform_params[ib.params_idx];
      wave = m_waveform_gen.generate_waveform(ib.waveform, note->duration_ms*1e-3f, note->frequency, params, 44100, false, ib.freq_effect, ib.ampl_effect, ib.phase_effect);
      
      return wave;
    }
    
    Waveform create_instrument_ring_mod(Note* note, const InstrumentRingMod& irm)
    {
      Waveform wave;
      
      Waveform wave_A = create_waveform(note, irm.ring_mod_instr_name_A);
      Waveform wave_B = create_waveform(note, irm.ring_mod_instr_name_B);
      
      wave = WaveformHelper::ring_modulation(wave_A, wave_B);
      
      return wave;
    }
    
    Waveform create_instrument_conv(Note* note, const InstrumentConv& ic)
    {
      Waveform wave;
      
      Waveform wave_A = create_waveform(note, ic.conv_instr_name_A);
      Waveform wave_B = create_waveform(note, ic.conv_instr_name_B);
      
      wave = WaveformHelper::reverb_fast(wave_A, wave_B);
      
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
      
      return wave;
    }
    
    void print_lib_instrument_type(InstrumentType lib_instrument)
    {
      std::string instrument;
      switch (lib_instrument)
      {
        case InstrumentType::PIANO: instrument = "PIANO"; break;
        case InstrumentType::VIOLIN: instrument = "VIOLIN"; break;
        case InstrumentType::ORGAN: instrument = "ORGAN"; break;
        case InstrumentType::TRUMPET: instrument = "TRUMPET"; break;
        case InstrumentType::FLUTE: instrument = "FLUTE"; break;
        case InstrumentType::GUITAR: instrument = "GUITAR"; break;
        case InstrumentType::KICKDRUM: instrument = "KICKDRUM"; break;
        case InstrumentType::SNAREDRUM: instrument = "SNAREDRUM"; break;
        case InstrumentType::HIHAT: instrument = "HIHAT"; break;
        case InstrumentType::ANVIL: instrument = "ANVIL"; break;
        default: break;
      }
      std::cout << "Instrument: " << instrument << std::endl;
    }
    
    Waveform create_instrument_lib(Note* note, const InstrumentLib& il)
    {
      Waveform wave;
      
      wave = Synthesizer::synthesize(il.lib_instrument, m_waveform_gen,
        note->duration_ms * 1e-3f, note->frequency, 44100, false,
        il.freq_effect, il.ampl_effect, il.phase_effect);
      
      return wave;
    }
    
    void apply_post_effects(Waveform& wave, int flt_idx, int adsr_idx)
    {
      if (flt_idx >= 0)
      {
        const auto& fa = m_filter_args[flt_idx];
        wave = WaveformHelper::filter(wave, fa);
      }
      if (adsr_idx >= 0)
      {
        const auto& adsr = m_envelopes[adsr_idx];
        wave = WaveformHelper::envelope_adsr(wave, adsr);
      }
    }
    
    void create_instruments()
    {
      for (auto& voice : m_voices)
      {
        for (auto& note : voice.notes)
        {
          if (!note->pause)
          {
            if (note->instrument_basic_idx >= 0)
            {
              const auto& ib = m_instruments_basic[note->instrument_basic_idx];
              note->wave = create_instrument_basic(note.get(), ib);
              note->gain *= ib.gain;
              apply_post_effects(note->wave, ib.flt_idx, ib.adsr_idx);
            }
            else if (note->instrument_ring_mod_idx >= 0)
            {
              const auto& irm = m_instruments_ring_mod[note->instrument_ring_mod_idx];
              note->wave = create_instrument_ring_mod(note.get(), irm);
              note->gain *= irm.gain;
              apply_post_effects(note->wave, irm.flt_idx, irm.adsr_idx);
            }
            else if (note->instrument_conv_idx >= 0)
            {
              const auto& ic = m_instruments_conv[note->instrument_conv_idx];
              note->wave = create_instrument_conv(note.get(), ic);
              note->gain *= ic.gain;
              apply_post_effects(note->wave, ic.flt_idx, ic.adsr_idx);
            }
            else if (note->instrument_weight_avg_idx >= 0)
            {
              const auto& iwa = m_instruments_weight_avg[note->instrument_weight_avg_idx];
              note->wave = create_instrument_weight_avg(note.get(), iwa);
              note->gain *= iwa.gain;
              apply_post_effects(note->wave, iwa.flt_idx, iwa.adsr_idx);
            }
            else if (note->instrument_lib_idx >= 0)
            {
              const auto& it = m_instruments_lib[note->instrument_lib_idx];
              note->wave = create_instrument_lib(note.get(), it);
              note->gain *= it.gain;
              apply_post_effects(note->wave, it.flt_idx, it.adsr_idx);
            }
            
            apply_post_effects(note->wave, note->flt_idx, note->adsr_idx);
          }
        }
      }
    }
    
    void init_voice_sources()
    {
      for (auto& voice : m_voices)
        voice.src = m_audio_handler.create_source();
    }
    
  };

}
