
#pragma once

//
// ogg vorbisデータを扱う
//

#if defined (_MSC_VER)
#ifdef _DEBUG
#pragma comment (lib, "liboggd_static.lib")
#pragma comment (lib, "libvorbisd_static.lib")
#pragma comment (lib, "libvorbisfiled_static.lib")
#else
#pragma comment (lib, "libogg_static.lib")
#pragma comment (lib, "libvorbis_static.lib")
#pragma comment (lib, "libvorbisfile_static.lib")
#endif
#endif

#include <string>
#include <vector>
#include <vorbis/vorbisfile.h>
#include <boost/algorithm/string.hpp>
#include "co_fileUtil.hpp"


namespace ngs {

class Ogg {
  u_int ch_;
  u_int sample_rate_;
  u_int size_;
  float time_;
  std::vector<char> data_;

  enum { BIT_NUM = 2 };
  // 量子化ビット数は16

public:
  explicit Ogg(const std::string& file) {
    OggVorbis_File fstr;
    ov_fopen(file.c_str(), &fstr);

    // 情報収集
    vorbis_info* info = ov_info(&fstr, -1);
    ch_ = info->channels;
    sample_rate_ = static_cast<u_int>(info->rate);
    size_ = ch_ * BIT_NUM * static_cast<u_int>(ov_pcm_total(&fstr, -1));

    // 読み込み
    data_.resize(size_);
    int bitstream = 0;

    // バッファを満たすまでデータを読み込む
    size_t remain_size = size_;
    size_t offset = 0;
    while (remain_size > 0) {
      size_t read_size = ov_read(&fstr, &data_[offset], static_cast<int>(remain_size), 0, BIT_NUM, 1, &bitstream);
      remain_size -= read_size;
      offset += read_size;
    }
    
    ov_clear(&fstr);
  }

	u_int channel() const { return ch_; }
  bool isStereo() const { return ch_ == 2; }
	u_int sampleRate() const { return sample_rate_; }
	u_int size() const { return size_; }
  float time() const { return time_; }
	const char* data() const { return &data_[0]; }
  
  // ファイルがogg vorbisか判別
  static bool isOgg(const std::string& file) {
    std::string file_ext = getFileExt(file);
    std::string ogg_ext(".ogg");
    return boost::iequals(file_ext, ogg_ext);
  }
  
};

}
