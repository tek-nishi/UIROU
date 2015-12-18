
#pragma once

//
// wavのストリーミング
// TODO:エラー処理
//

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "co_wav.hpp"
#include "co_streamBase.hpp"


namespace ngs {

class StreamWav :  public StreamBase {
  std::ifstream fstr_;

  Wav::Info info;
  size_t top_pos_;

  bool loop_;

  size_t last_size_;

  
  // ファイルからバッファへデータを読み込む
  size_t readData(std::vector<char>& buffer, const size_t offset, const size_t size) {
    size_t read_size = (size < last_size_) ? size : last_size_;
    fstr_.read(&buffer[offset], read_size);
    return read_size;
  }
  
public:
  explicit StreamWav(const std::string& file) :
    fstr_(file, std::ios::binary),
    loop_(false)
  {
    DOUT << "StreamWav()" << std::endl;

		if (fstr_) {
      // ファイル情報を解析
      if (!Wav::analyzeWavFile(info, fstr_)) {
        return;
      }

      if ((info.id != 1) || (info.bit != 16)) {
        // IDが１で量子化ビット数が16以外は扱わない
        DOUT << "Wav format error. " << info.id << " " << info.bit << std::endl;
        return;
      }

      last_size_ = info.size;
      top_pos_ = static_cast<size_t>(fstr_.tellg());
      // データの先頭位置を覚えておく
		}
  }

  ~StreamWav() {
    DOUT << "~StreamWav()" << std::endl;
  }

  
  bool isStereo() const { return info.ch == 2; }
  u_int sampleRate() const { return info.sample_rate; }
  
  void loop(const bool loop) {
    loop_ = loop;
  }

  // 再生位置を先頭に戻す
  void toTop() {
    fstr_.seekg(top_pos_, fstr_.beg);
    last_size_ = info.size;
  }

  bool isEnd() const { return last_size_ == 0; }
  
  // 再生バッファにデータを読み込む
  size_t read(std::vector<char>& buffer) {
    size_t remain_size = buffer.size();
    if (!loop_ && (last_size_ < remain_size)) remain_size = last_size_;
    // ループしない場合、残りの中途半端なサイズを読み込んで終了

    size_t offset = 0;
    size_t total_read_size = 0;

    // ループ再生の場合はバッファを満たすまでデータを読み込む
    while (remain_size > 0) {
      size_t read_size = readData(buffer, offset, remain_size);
      total_read_size += read_size;
      remain_size -= read_size;
      last_size_ -= read_size;
      offset += read_size;
      if (loop_ && !last_size_) toTop();
    }
    
    return total_read_size;
  }
  
};

}
