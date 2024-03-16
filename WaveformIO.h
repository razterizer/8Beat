//
//  WaveformIO.h
//  8-Bit Audio Emulator Lib
//
//  Created by Rasmus Anthin on 2024-02-16.
//

#pragma once

#include "Waveform.h"
#include "WaveformHelper.h"

#include <sndfile.h>
//#include <typeinfo>
#include <type_traits>

#define SUBTYPE_ITEM(sti) sti = SF_FORMAT_##sti


namespace audio
{
  enum class AudioFileFormatSubType : int
  {
    SUBTYPE_ITEM(PCM_S8), // Signed 8 bit data
    SUBTYPE_ITEM(PCM_16), // Signed 16 bit data
    SUBTYPE_ITEM(PCM_24), // Signed 24 bit data
    SUBTYPE_ITEM(PCM_32), // Signed 32 bit data
    SUBTYPE_ITEM(PCM_U8), // Unsigned 8 bit data (WAV and RAW only)
    SUBTYPE_ITEM(FLOAT), // 32 bit float data
    SUBTYPE_ITEM(DOUBLE), // 64 bit float data
    SUBTYPE_ITEM(ULAW), // U-Law encoded
    SUBTYPE_ITEM(ALAW), // A-Law encoded
    SUBTYPE_ITEM(IMA_ADPCM), // IMA ADPCM
    SUBTYPE_ITEM(MS_ADPCM), // Microsoft ADPCM
    SUBTYPE_ITEM(GSM610), // GSM 6.10 encoding
    SUBTYPE_ITEM(VOX_ADPCM), // Oki Dialogic ADPCM encoding
    SUBTYPE_ITEM(G721_32), // 32kbs G721 ADPCM encoding
    SUBTYPE_ITEM(G723_24), // 24kbs G723 ADPCM encoding
    SUBTYPE_ITEM(G723_40), // 40kbs G723 ADPCM encoding
    SUBTYPE_ITEM(DWVW_12), // 12 bit Delta Width Variable Word encoding
    SUBTYPE_ITEM(DWVW_16), // 16 bit Delta Width Variable Word encoding
    SUBTYPE_ITEM(DWVW_24), // 24 bit Delta Width Variable Word encoding
    SUBTYPE_ITEM(DWVW_N), // N bit Delta Width Variable Word encoding
    SUBTYPE_ITEM(DPCM_8), // 8 bit differential PCM (XI only)
    SUBTYPE_ITEM(DPCM_16), // 16 bit differential PCM (XI only)
    SUBTYPE_ITEM(VORBIS), // Xiph Vorbis encoding
    SUBTYPE_ITEM(OPUS), // Xiph/Skype Opus encoding
  };

  class WaveformIO
  {
  public:
    static std::optional<Waveform> load(const std::string& filepath,
      int verbosity = 1)
    {
      Waveform wd;
      
      SF_INFO sf_info;
      sf_info.format = 0; // #NOTE: Specify for format raw.
      SNDFILE* file = sf_open(filepath.c_str(), SFM_READ, &sf_info);
      
      if (verbosity >= 3)
        print_format(sf_info);
      
      if (file == nullptr)
      {
        if (verbosity >= 1)
          std::cerr << "ERROR: File Not Found or Invalid Format.\n";
        return {};
      }
      else
      {
        if (verbosity >= 2)
          std::cout << "Successfully loaded WaveForm from file: \"" << filepath << "\".\n";
      }
      
      wd.sample_rate = static_cast<float>(sf_info.samplerate);
      wd.duration = static_cast<float>(sf_info.frames) / wd.sample_rate;
      //std::cout << "Fs: " << wd.sample_rate << std::endl;
      //std::cout << "duration: " << wd.duration << std::endl;
      //std::cout << "channels: " << sf_info.channels << std::endl;
      
      const size_t input_buf_size = sf_info.channels * sf_info.frames;
      std::vector<float> buffer_in(input_buf_size);
      wd.buffer.resize(sf_info.frames);
      
      // #FIXME: Fix proper stereo-support in the future.
      sf_read_float(file, buffer_in.data(), static_cast<sf_count_t>(input_buf_size));
      for (size_t f_idx = 0; f_idx < sf_info.frames; ++f_idx)
      {
        wd.buffer[f_idx] = 0;
        for (int c_idx = 0; c_idx < sf_info.channels; c_idx++)
            wd.buffer[f_idx] += buffer_in[f_idx*sf_info.channels + c_idx];
        wd.buffer[f_idx] /= sf_info.channels;
      }
      // Normalize to amplitude max 1 if amplitude > 1.
      WaveformHelper::normalize_over(wd);
      
      if (verbosity >= 2)
        std::cout << "Channel mixing complete!\n";
      
      sf_close(file);
      
      return wd;
    }
    
    static bool save(const Waveform& wd, const std::string& filepath,
                     AudioFileFormatSubType subtype,
                     int verbosity = 1)
    {
      SF_INFO sf_info;
      
      sf_info.samplerate = static_cast<int>(wd.sample_rate);
      sf_info.frames = static_cast<sf_count_t>(wd.buffer.size());
      sf_info.channels = 1;  // Use 1 for mono, update for stereo support.
      
      auto filepath2 = filepath;
      auto idx = filepath2.rfind('.');
      if (idx == std::string::npos)
      {
        filepath2 += ".wav";
        idx = filepath2.rfind('.');
        if (idx == std::string::npos)
        {
          if (verbosity >= 1)
            std::cerr << "ERROR: Unable to figure out the file extension!\n";
          return false;
        }
      }
      auto ext = filepath2.substr(idx + 1);
      if (ext == "wav") sf_info.format = SF_FORMAT_WAV;
      else if (ext == "aiff") sf_info.format = SF_FORMAT_AIFF;
      else if (ext == "au") sf_info.format = SF_FORMAT_AU;
      else if (ext == "raw") sf_info.format = SF_FORMAT_RAW;
      else if (ext == "paf") sf_info.format = SF_FORMAT_PAF;
      else if (ext == "svx") sf_info.format = SF_FORMAT_SVX;
      else if (ext == "nist") sf_info.format = SF_FORMAT_NIST;
      else if (ext == "voc") sf_info.format = SF_FORMAT_VOC;
      else if (ext == "ircam") sf_info.format = SF_FORMAT_IRCAM;
      else if (ext == "w64") sf_info.format = SF_FORMAT_W64;
      else if (ext == "mat4") sf_info.format = SF_FORMAT_MAT4;
      else if (ext == "mat5") sf_info.format = SF_FORMAT_MAT5;
      else if (ext == "pvf") sf_info.format = SF_FORMAT_PVF;
      else if (ext == "xi") sf_info.format = SF_FORMAT_XI;
      else if (ext == "htk") sf_info.format = SF_FORMAT_HTK;
      else if (ext == "sds") sf_info.format = SF_FORMAT_SDS;
      else if (ext == "avr") sf_info.format = SF_FORMAT_AVR;
      else if (ext == "wavex") sf_info.format = SF_FORMAT_WAVEX;
      else if (ext == "sd2") sf_info.format = SF_FORMAT_SD2;
      else if (ext == "flac") sf_info.format = SF_FORMAT_FLAC;
      else if (ext == "caf") sf_info.format = SF_FORMAT_CAF;
      else if (ext == "wve") sf_info.format = SF_FORMAT_WVE;
      else if (ext == "ogg") sf_info.format = SF_FORMAT_OGG;
      else if (ext == "mpc2k") sf_info.format = SF_FORMAT_MPC2K;
      else if (ext == "rf64") sf_info.format = SF_FORMAT_RF64;
      
      
      bool correct_subtype = false;
      switch (sf_info.format)
      {
        case SF_FORMAT_WAV:
          if (subtype == AudioFileFormatSubType::PCM_16
              || subtype == AudioFileFormatSubType::PCM_24
              || subtype == AudioFileFormatSubType::PCM_32
              || subtype == AudioFileFormatSubType::PCM_U8
              || subtype == AudioFileFormatSubType::FLOAT
              || subtype == AudioFileFormatSubType::DOUBLE
              || subtype == AudioFileFormatSubType::ULAW
              || subtype == AudioFileFormatSubType::ALAW
              || subtype == AudioFileFormatSubType::IMA_ADPCM
              || subtype == AudioFileFormatSubType::MS_ADPCM
              || subtype == AudioFileFormatSubType::GSM610
              || subtype == AudioFileFormatSubType::VOX_ADPCM
              || subtype == AudioFileFormatSubType::G721_32
              || subtype == AudioFileFormatSubType::G723_24
              || subtype == AudioFileFormatSubType::G723_40
              || subtype == AudioFileFormatSubType::DWVW_12
              || subtype == AudioFileFormatSubType::DWVW_16
              || subtype == AudioFileFormatSubType::DWVW_24
              || subtype == AudioFileFormatSubType::DWVW_N
              || subtype == AudioFileFormatSubType::DPCM_8
              || subtype == AudioFileFormatSubType::DPCM_16
              || subtype == AudioFileFormatSubType::VORBIS)
          {
            correct_subtype = true;
          }
          break;
        case SF_FORMAT_AIFF:
        case SF_FORMAT_AU:
        case SF_FORMAT_RAW:
        case SF_FORMAT_FLAC:
        case SF_FORMAT_SD2:
        case SF_FORMAT_RF64:
        case SF_FORMAT_MAT4:
        case SF_FORMAT_MAT5:
        case SF_FORMAT_SVX:
        case SF_FORMAT_VOC:
        case SF_FORMAT_IRCAM:
        case SF_FORMAT_W64:
        case SF_FORMAT_PVF:
        case SF_FORMAT_XI:
        case SF_FORMAT_HTK:
        case SF_FORMAT_SDS:
        case SF_FORMAT_AVR:
        case SF_FORMAT_MPC2K:
          if (subtype == AudioFileFormatSubType::PCM_S8
              || subtype == AudioFileFormatSubType::PCM_16
              || subtype == AudioFileFormatSubType::PCM_24
              || subtype == AudioFileFormatSubType::PCM_32
              || subtype == AudioFileFormatSubType::PCM_U8
              || subtype == AudioFileFormatSubType::FLOAT
              || subtype == AudioFileFormatSubType::DOUBLE)
          {
            correct_subtype = true;
          }
          break;
        case SF_FORMAT_CAF:
        case SF_FORMAT_WVE:
          if (subtype == AudioFileFormatSubType::PCM_S8
              || subtype == AudioFileFormatSubType::PCM_16
              || subtype == AudioFileFormatSubType::PCM_24
              || subtype == AudioFileFormatSubType::PCM_32
              || subtype == AudioFileFormatSubType::PCM_U8
              || subtype == AudioFileFormatSubType::FLOAT
              || subtype == AudioFileFormatSubType::DOUBLE
              || subtype == AudioFileFormatSubType::ULAW
              || subtype == AudioFileFormatSubType::ALAW
              || subtype == AudioFileFormatSubType::IMA_ADPCM
              || subtype == AudioFileFormatSubType::MS_ADPCM
              || subtype == AudioFileFormatSubType::G723_24
              || subtype == AudioFileFormatSubType::G723_40
              || subtype == AudioFileFormatSubType::OPUS)
          {
            correct_subtype = true;
          }
          break;
        case SF_FORMAT_WAVEX:
        case SF_FORMAT_PAF:
        case SF_FORMAT_NIST:
          if (subtype == AudioFileFormatSubType::PCM_16
              || subtype == AudioFileFormatSubType::PCM_24
              || subtype == AudioFileFormatSubType::PCM_32
              || subtype == AudioFileFormatSubType::PCM_U8
              || subtype == AudioFileFormatSubType::FLOAT
              || subtype == AudioFileFormatSubType::DOUBLE)
          {
            correct_subtype = true;
          }
          break;
        case SF_FORMAT_OGG:
          if (subtype == AudioFileFormatSubType::VORBIS)
          {
            correct_subtype = true;
          }
          break;
      }
      
      auto subtype_str = get_format_subtype_str(subtype);
      if (!correct_subtype)
      {
        if (verbosity >= 1)
          std::cerr << "ERROR: Unsupported format subtype " << subtype_str << " for major format " << ext << "." << std::endl;
        return false;
      }
      
      sf_info.format |= static_cast<int>(subtype);
      //std::cout << static_cast<int>(subtype) << std::endl;
      
      if (verbosity >= 3)
        print_format(sf_info);
      
      SNDFILE* file = sf_open(filepath2.c_str(), SFM_WRITE, &sf_info);
      
      if (file == nullptr)
      {
        if (verbosity >= 1)
          std::cerr << "ERROR: Unable to create or write to file: \"" << filepath2 << "\"\n";
        return false;
      }
      
      sf_info.samplerate = static_cast<int>(wd.sample_rate);
      sf_info.frames = static_cast<sf_count_t>(wd.buffer.size());
      sf_info.channels = 1;  // Use 1 for mono, update for stereo support.
      
      //std::cout << "sf_info.channels: " << sf_info.channels << std::endl;
      //std::cout << "sf_info.frames: " << sf_info.channels << std::endl;
      //std::cout << "wd.buffer.size(): " << wd.buffer.size() << std::endl;
      
      sf_count_t written_frames = 0;
      switch (subtype)
      {
        case AudioFileFormatSubType::PCM_S8:
          written_frames = write_PCM<int8_t>(file, wd);
          break;
        case AudioFileFormatSubType::PCM_16:
        case AudioFileFormatSubType::FLOAT:
          written_frames = write_PCM_float(file, wd);
          break;
        case AudioFileFormatSubType::DOUBLE:
          written_frames = write_PCM_double(file, wd);
          break;
        case AudioFileFormatSubType::PCM_24:
        case AudioFileFormatSubType::PCM_32:
          written_frames = write_PCM<int32_t>(file, wd);
          break;
        case AudioFileFormatSubType::PCM_U8:
          written_frames = write_PCM<uint8_t>(file, wd);
          break;
        case AudioFileFormatSubType::ULAW:
          written_frames = write_ULAW(file, wd);
          break;
        case AudioFileFormatSubType::ALAW:
          written_frames = write_ALAW(file, wd);
          break;
        case AudioFileFormatSubType::IMA_ADPCM:
        case AudioFileFormatSubType::MS_ADPCM:
        case AudioFileFormatSubType::GSM610:
        case AudioFileFormatSubType::VOX_ADPCM:
        case AudioFileFormatSubType::G721_32:
        case AudioFileFormatSubType::G723_24:
        case AudioFileFormatSubType::G723_40:
        case AudioFileFormatSubType::DWVW_12:
        case AudioFileFormatSubType::DWVW_16:
        case AudioFileFormatSubType::DWVW_24:
        case AudioFileFormatSubType::DWVW_N:
        case AudioFileFormatSubType::DPCM_8:
        case AudioFileFormatSubType::DPCM_16:
        case AudioFileFormatSubType::VORBIS:
        case AudioFileFormatSubType::OPUS:
          if (verbosity >= 1)
            std::cerr << "ERROR: Subtype " << subtype_str << " is not yet implemented." << std::endl;
          return false;
        default:
          if (verbosity >= 1)
            std::cerr << "ERROR: Unknown subtype encountered." << std::endl;
          return false;
      }
      
      sf_close(file);
      
      if (written_frames != sf_info.frames)
      {
        if (verbosity >= 1)
          std::cerr << "ERROR: Failed to write the expected number of frames to file: \"" << filepath2 << "\"\n";
        return false;
      }
      
      if (verbosity >= 2)
        std::cout << "Successfully saved WaveForm to file: \"" << filepath2 << "\".\n";
      return true;
    }
    
  private:
    static void print_format(const SF_INFO& sf_info)
    {
      auto test_mask_format = [&sf_info](int mask)
      {
        return (sf_info.format & 0x0FFF0000) == mask;
      };
      
      auto test_mask_subtype = [&sf_info](int mask)
      {
        return (sf_info.format & 0x0000FFFF) == mask;
      };
      
      auto test_mask_endian = [&sf_info](int mask)
      {
        return (sf_info.format & 0x30000000) == mask;
      };
      
      std::cout << "Major Format: ";
      if (test_mask_format(SF_FORMAT_WAV)) std::cout << "wav";
      else if (test_mask_format(SF_FORMAT_AIFF)) std::cout << "aiff";
      else if (test_mask_format(SF_FORMAT_AU)) std::cout << "au";
      else if (test_mask_format(SF_FORMAT_RAW)) std::cout << "raw";
      else if (test_mask_format(SF_FORMAT_PAF)) std::cout << "paf";
      else if (test_mask_format(SF_FORMAT_SVX)) std::cout << "svx";
      else if (test_mask_format(SF_FORMAT_NIST)) std::cout << "nist";
      else if (test_mask_format(SF_FORMAT_VOC)) std::cout << "voc";
      else if (test_mask_format(SF_FORMAT_IRCAM)) std::cout << "ircam";
      else if (test_mask_format(SF_FORMAT_W64)) std::cout << "w64";
      else if (test_mask_format(SF_FORMAT_MAT4)) std::cout << "mat4";
      else if (test_mask_format(SF_FORMAT_MAT5)) std::cout << "mat5";
      else if (test_mask_format(SF_FORMAT_PVF)) std::cout << "pvf";
      else if (test_mask_format(SF_FORMAT_XI)) std::cout << "xi";
      else if (test_mask_format(SF_FORMAT_HTK)) std::cout << "htk";
      else if (test_mask_format(SF_FORMAT_SDS)) std::cout << "sds";
      else if (test_mask_format(SF_FORMAT_AVR)) std::cout << "avr";
      else if (test_mask_format(SF_FORMAT_WAVEX)) std::cout << "wavex";
      else if (test_mask_format(SF_FORMAT_SD2)) std::cout << "sd2";
      else if (test_mask_format(SF_FORMAT_FLAC)) std::cout << "flac";
      else if (test_mask_format(SF_FORMAT_CAF)) std::cout << "caf";
      else if (test_mask_format(SF_FORMAT_WVE)) std::cout << "wve";
      else if (test_mask_format(SF_FORMAT_OGG)) std::cout << "ogg";
      else if (test_mask_format(SF_FORMAT_MPC2K)) std::cout << "mpc2k";
      else if (test_mask_format(SF_FORMAT_RF64)) std::cout << "rf64";
      std::cout << std::endl;
      
      std::cout << "Subtype: ";
      if (test_mask_subtype(SF_FORMAT_PCM_S8)) std::cout << "Signed 8 bit data";
      else if (test_mask_subtype(SF_FORMAT_PCM_16)) std::cout << "Signed 16 bit data";
      else if (test_mask_subtype(SF_FORMAT_PCM_24)) std::cout << "Signed 24 bit data";
      else if (test_mask_subtype(SF_FORMAT_PCM_32)) std::cout << "Signed 32 bit data";
      else if (test_mask_subtype(SF_FORMAT_PCM_U8)) std::cout << "Unsigned 8 bit data (WAV and RAW only)";
      else if (test_mask_subtype(SF_FORMAT_FLOAT)) std::cout << "32 bit float data";
      else if (test_mask_subtype(SF_FORMAT_DOUBLE)) std::cout << "64 bit float data";
      else if (test_mask_subtype(SF_FORMAT_ULAW)) std::cout << "U-Law encoded";
      else if (test_mask_subtype(SF_FORMAT_ALAW)) std::cout << "A-Law encoded";
      else if (test_mask_subtype(SF_FORMAT_IMA_ADPCM)) std::cout << "IMA ADPCM";
      else if (test_mask_subtype(SF_FORMAT_MS_ADPCM)) std::cout << "Microsoft ADPCM";
      else if (test_mask_subtype(SF_FORMAT_GSM610)) std::cout << "GSM 6.10 encoding";
      else if (test_mask_subtype(SF_FORMAT_VOX_ADPCM)) std::cout << "Oki Dialogic ADPCM encoding";
      else if (test_mask_subtype(SF_FORMAT_G721_32)) std::cout << "32kbs G721 ADPCM encoding";
      else if (test_mask_subtype(SF_FORMAT_G723_24)) std::cout << "24kbs G723 ADPCM encoding";
      else if (test_mask_subtype(SF_FORMAT_G723_40)) std::cout << "40kbs G723 ADPCM encoding";
      else if (test_mask_subtype(SF_FORMAT_DWVW_12)) std::cout << "12 bit Delta Width Variable Word encoding";
      else if (test_mask_subtype(SF_FORMAT_DWVW_16)) std::cout << "16 bit Delta Width Variable Word encoding";
      else if (test_mask_subtype(SF_FORMAT_DWVW_24)) std::cout << "24 bit Delta Width Variable Word encoding";
      else if (test_mask_subtype(SF_FORMAT_DWVW_N)) std::cout << "N bit Delta Width Variable Word encoding";
      else if (test_mask_subtype(SF_FORMAT_DPCM_8)) std::cout << "8 bit differential PCM (XI only)";
      else if (test_mask_subtype(SF_FORMAT_DPCM_16)) std::cout << "16 bit differential PCM (XI only)";
      else if (test_mask_subtype(SF_FORMAT_VORBIS)) std::cout << "Xiph Vorbis encoding";
      else if (test_mask_subtype(SF_FORMAT_OPUS)) std::cout << "Xiph/Skype Opus encoding";
      std::cout << std::endl;
      
      std::cout << "Endian-ness Options: ";
      if (test_mask_endian(SF_ENDIAN_FILE)) std::cout << "Default file endian-ness";
      else if (test_mask_endian(SF_ENDIAN_LITTLE)) std::cout << "Force little endian-ness";
      else if (test_mask_endian(SF_ENDIAN_BIG)) std::cout << "Force big endian-ness";
      else if (test_mask_endian(SF_ENDIAN_CPU)) std::cout << "Force CPU endian-ness";
      std::cout << std::endl;
    }
    
    static std::string get_format_subtype_str(AudioFileFormatSubType subtype)
    {
      switch (subtype)
      {
        case AudioFileFormatSubType::PCM_S8: return "PCM_S8"; // Signed 8 bit data
        case AudioFileFormatSubType::PCM_16: return "PCM_16"; // Signed 16 bit data
        case AudioFileFormatSubType::PCM_24: return "PCM_24"; // Signed 24 bit data
        case AudioFileFormatSubType::PCM_32: return "PCM_32"; // Signed 32 bit data
        case AudioFileFormatSubType::PCM_U8: return "PCM_U8"; // Unsigned 8 bit data (WAV and RAW only)
        case AudioFileFormatSubType::FLOAT: return "FLOAT"; // 32 bit float data
        case AudioFileFormatSubType::DOUBLE: return "DOUBLE"; // 64 bit float data
        case AudioFileFormatSubType::ULAW: return "ULAW"; // U-Law encoded
        case AudioFileFormatSubType::ALAW: return "ALAW"; // A-Law encoded
        case AudioFileFormatSubType::IMA_ADPCM: return "IMA_ADPCM"; // IMA ADPCM
        case AudioFileFormatSubType::MS_ADPCM: return "MS_ADPCM"; // Microsoft ADPCM
        case AudioFileFormatSubType::GSM610: return "GSM610"; // GSM 6.10 encoding
        case AudioFileFormatSubType::VOX_ADPCM: return "VOX_ADPCM"; // Oki Dialogic ADPCM encoding
        case AudioFileFormatSubType::G721_32: return "G721_32"; // 32kbs G721 ADPCM encoding
        case AudioFileFormatSubType::G723_24: return "G723_24"; // 24kbs G723 ADPCM encoding
        case AudioFileFormatSubType::G723_40: return "G723_40"; // 40kbs G723 ADPCM encoding
        case AudioFileFormatSubType::DWVW_12: return "DWVW_12"; // 12 bit Delta Width Variable Word encoding
        case AudioFileFormatSubType::DWVW_16: return "DWVW_16"; // 16 bit Delta Width Variable Word encoding
        case AudioFileFormatSubType::DWVW_24: return "DWVW_24"; // 24 bit Delta Width Variable Word encoding
        case AudioFileFormatSubType::DWVW_N: return "DWVW_N"; // N bit Delta Width Variable Word encoding
        case AudioFileFormatSubType::DPCM_8: return "DPCM_8"; // 8 bit differential PCM (XI only)
        case AudioFileFormatSubType::DPCM_16: return "DPCM_16"; // 16 bit differential PCM (XI only)
        case AudioFileFormatSubType::VORBIS: return "VORBIS"; // Xiph Vorbis encoding
        case AudioFileFormatSubType::OPUS: return "OPUS"; // Xiph/Skype Opus encoding
      }
    }
    
    template <typename T>
    static sf_count_t write_PCM(SNDFILE* file, const Waveform& wd)
    {
      // Convert float data to the specified bit depth
      std::vector<T> buffer(wd.buffer.size());
      const auto min_val = static_cast<float>(std::numeric_limits<T>::min());
      const auto max_val = static_cast<float>(std::numeric_limits<T>::max());
      const float scale_factor = max_val;
      
      for (size_t i = 0; i < wd.buffer.size(); ++i)
      {
        auto scaled_value = wd.buffer[i] * scale_factor;
        buffer[i] = static_cast<T>(std::max(min_val, std::min(max_val, scaled_value)));
      }
      
      // Write data to file
      sf_count_t written_frames = 0;
      if constexpr (std::is_same_v<T, int8_t> || std::is_same_v<T, uint8_t>)
        written_frames = sf_write_raw(file, buffer.data(), buffer.size());
      else
        written_frames = sf_write_int(file, buffer.data(), buffer.size());
      
      return written_frames;
    }
    
    static sf_count_t write_PCM_float(SNDFILE* file, const Waveform& wd)
    {
      // Write float data to file
      sf_count_t written_frames = sf_write_float(file, wd.buffer.data(), wd.buffer.size());
      return written_frames;
    }
    
    static sf_count_t write_PCM_double(SNDFILE* file, const Waveform& wd)
    {
      // Convert float data to double
      std::vector<double> buffer_double(wd.buffer.begin(), wd.buffer.end());
      
      // Write double data to file
      sf_count_t written_frames = sf_write_double(file, buffer_double.data(), buffer_double.size());
      return written_frames;
    }
    
    // From ChatGPT.
    static uint8_t linear_to_ulaw(float sample)
    {
      const float bias = 132.0f;
      const float max_value = 32768.0f;
      const float uLawMax = 32768.0f;
      
      // Ensure the sample is within the valid range
      if (sample > max_value)
        sample = max_value;
      else if (sample < -max_value)
        sample = -max_value;
      
      // Apply bias and scale
      int scaledValue = static_cast<int>(sample + bias);
      
      // Map the linear value to the u-law range
      if (scaledValue < 0)
        scaledValue = 0;
      else if (scaledValue > uLawMax)
        scaledValue = uLawMax;
      
      // Quantize to 8 bits
      int exponent = 7;
      int mantissa = 0;
      do
      {
        mantissa = (scaledValue >> exponent) & 1;
        exponent--;
      } while (exponent >= 0 && mantissa == 0);
      
      int uLawValue = (exponent << 4) | ((7 - exponent) << 3) | (mantissa << 3) | ((scaledValue >> (exponent + 3)) & 7);
      
      return static_cast<uint8_t>(uLawValue);
    }
    
    // From ChatGPT.
    static uint8_t linear_to_alaw(float sample)
    {
      const float max_value = 32768.0f;
      
      // Ensure the sample is within the valid range
      if (sample > max_value)
        sample = max_value;
      else if (sample < -max_value)
        sample = -max_value;
      
      // Perform A-law compression
      int sign = (sample < 0) ? 0x80 : 0x00;
      int exponent = 0, mantissa = 0;
      
      sample = std::fabs(sample);
      if (sample >= 0.0030517578125) // |sample| >= 1/32768
      {
        if (sample < 1.52587890625) // |sample| < 32/32768
        {
          exponent = static_cast<int>(log(sample) / log(2) + 132);
          mantissa = static_cast<int>(sample * pow(2, 7 - exponent) + 0.5) & 0x0F;
        }
        else
        {
          exponent = 0x0F;
          mantissa = 0x0F;
        }
      }
      else
      {
        exponent = static_cast<int>(sample / 0.0030517578125) + 0x30;
        mantissa = static_cast<int>((sample / pow(2, 7 - (exponent & 0x0F))) + 0.5) & 0x0F;
      }
      
      return static_cast<uint8_t>(sign | ((exponent & 0x0F) << 4) | mantissa);
    }
    
    static sf_count_t write_ULAW(SNDFILE* file, const Waveform& wd)
    {
      // Convert float data to μ-law encoded integers
      std::vector<uint8_t> buffer_ulaw(wd.buffer.size());
      
      for (size_t i = 0; i < wd.buffer.size(); ++i)
      {
        // Convert float value to μ-law
        buffer_ulaw[i] = linear_to_ulaw(wd.buffer[i]);
      }
      
      // Write μ-law data to file
      sf_count_t written_frames = sf_write_raw(file, buffer_ulaw.data(), buffer_ulaw.size());
      
      return written_frames;
    }
    
    static sf_count_t write_ALAW(SNDFILE* file, const Waveform& wd)
    {
      // Convert float data to A-law encoded integers
      std::vector<uint8_t> buffer_alaw(wd.buffer.size());
      
      for (size_t i = 0; i < wd.buffer.size(); ++i)
      {
        // Convert float value to A-law
        buffer_alaw[i] = linear_to_alaw(wd.buffer[i]);
      }
      
      // Write A-law data to file
      sf_count_t written_frames = sf_write_raw(file, buffer_alaw.data(), buffer_alaw.size());
      
      return written_frames;
    }

    
  };

}
