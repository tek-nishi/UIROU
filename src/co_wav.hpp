
#pragma once

//
// wavデータを扱う
//

#include <string>
#include <vector>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include "co_fileUtil.hpp"
#include "co_misc.hpp"


namespace ngs {

class Wav {
public:
  struct Info {
    u_int id;
    u_int ch;
    u_int sample_rate;
    u_int bit;
    u_int size;
  };

private:
  Info info;
  float time_;
  std::vector<char> data_;

public:
  explicit Wav(const std::string& file) {
		std::ifstream fstr(file, std::ios::binary);
		if (fstr) {
      // ファイル情報を解析
      if (!Wav::analyzeWavFile(info, fstr)) {
        return;
      }

      if ((info.id != 1) || (info.bit != 16)) {
        // IDが１で量子化ビット数が16以外は扱わない
        DOUT << "Wav format error. " << info.id << " " << info.bit << std::endl;
        return;
      }

      // 再生時間(秒)
      time_ = info.size / info.ch / 2.0f / info.sample_rate;

      // データ読み込み
      data_.resize(info.size);
      searchChunk(fstr, "data");
      fstr.seekg(4, fstr.cur);      
      fstr.read(&data_[0], info.size);
    }
  }

	u_int channel() const { return info.ch; }
  bool isStereo() const { return info.ch == 2; }
	u_int sampleRate() const { return info.sample_rate; }
	u_int size() const { return info.size; }
  float time() const { return time_; }
	const char* data() const { return &data_[0]; }


  // wavの指定チャンクを探す
  static bool searchChunk(std::ifstream& fstr, const char* chunk) {
    enum {
      WAV_HEADER_SIZE = 12
      // チャンクが始まる位置
    };
    
    fstr.seekg(WAV_HEADER_SIZE, fstr.beg);
    // チャンクの並びは不定なので、常にファイルの先頭から探す

    while (1) {
      char tag[4];
      fstr.read(tag, 4);
      if (!std::strncmp(tag, chunk, 4)) {
        return true;
      }

      // 次のチャンクへ
      char data[4];
      fstr.read(data, 4);
      u_int chunk_size = getNum(data, 4);
      fstr.seekg(chunk_size, fstr.cur);
    
      if (fstr.eof()) break; 
    }
    return false;
  }

  // チャンクのサイズを取得
  static u_int getChunkSize(std::ifstream& fstr) {
    char data[4];
    fstr.read(data, 4);
    return getNum(data, 4);
  }

  // wavの情報を取得
  static bool analyzeWavFile(Info& info, std::ifstream& fstr) {
    // ファイルがwav形式か判別
    enum {
      WAV_HEADER_SIZE = 12
    };
    
    char header[WAV_HEADER_SIZE];
    fstr.read(header, WAV_HEADER_SIZE);
    if (std::strncmp(&header[0], "RIFF", 4)) {
      DOUT << "This file isn't RIFF format." << std::endl;
      return false;
    }
    if (std::strncmp(&header[8], "WAVE", 4)) {
      DOUT << "This file isn't WAVE format." << std::endl;
      return false;
    }
      
    enum {
      // fmtチャンク内のデータ位置
      WAV_ID          = 0,
      WAV_CH          = WAV_ID + 2,
      WAV_SAMPLE_RATE = WAV_CH + 2,
      WAV_BPS         = WAV_SAMPLE_RATE + 4,
      WAV_BLOCK_SIZE  = WAV_BPS + 4,
      WAV_BIT         = WAV_BLOCK_SIZE + 2,
    };

    // fmtチャンクを探してデータ形式を取得
    if (!searchChunk(fstr, "fmt ")) {
      DOUT << "No chank 'fmt'." << std::endl;
      return false;
    }
    u_int chunk_size = getChunkSize(fstr);
    std::vector<char> chunk(chunk_size);
    fstr.read(&chunk[0], chunk_size);

    info.id = getNum(&chunk[WAV_ID], 2);
    info.ch = getNum(&chunk[WAV_CH], 2);
    info.sample_rate = getNum(&chunk[WAV_SAMPLE_RATE], 4);
    info.bit = getNum(&chunk[WAV_BIT], 2);

    // dataチャンクを探してデータ長を取得
    if (!searchChunk(fstr, "data")) {
      DOUT << "No chank 'data'." << std::endl;
      return false;
    }
    info.size = getChunkSize(fstr);

    return true;
  }
  
  // ファイルがwavか判別
  static bool isWav(const std::string& file) {
    std::string file_ext = getFileExt(file);
    std::string wav_ext(".wav");
    return boost::iequals(file_ext, wav_ext);
  }
  
};

}
