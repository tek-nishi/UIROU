
#pragma once

//
// ストリーミング
// wav と ogg vorbis を自動判定して読み込み
//

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <boost/noncopyable.hpp>
#include "co_audio.hpp"
#include "co_wav.hpp"
#include "co_ogg.hpp"
#include "co_streamWav.hpp"
#include "co_streamOgg.hpp"


namespace ngs {

class Streaming : private boost::noncopyable {
  enum {
    BUFFER_NUM  = 2,
    BUFFER_SIZE = 1024 * 64
  };

  u_int index_;

  Audio::Buffer buffer_[BUFFER_NUM];
  Audio::Source source_;

  StreamBase* stream_;

  std::vector<char> sound_buffer_;
  // 読み込みバッファ

  bool pause_;
  

  // データを読み込んで再生キューに登録する
  void readStream() {
    if (!stream_->isEnd()) {
      size_t length = stream_->read(sound_buffer_);
      buffer_[index_].bind(stream_->isStereo(), &sound_buffer_[0], static_cast<u_int>(length), stream_->sampleRate());
      source_.queueBuffer(buffer_[index_]);
      index_ = (index_ + 1) % BUFFER_NUM;
    }
  }
  
public:
  Streaming(const std::string& file) :
    index_(0),
    stream_(0),
    sound_buffer_(BUFFER_SIZE),
    pause_(false)
  {
    DOUT << "Streaming()" << std::endl;

    // ogg vorbis か wav かを判別する
    if (Ogg::isOgg(file)) {
      stream_ = new StreamOgg(file);
      DOUT << "Streaming:read ogg vorbis" << std::endl;
    }
    else {
      stream_ = new StreamWav(file);
      DOUT << "Streaming:read wav" << std::endl;
    }
  }

  ~Streaming() {
    DOUT << "~Streaming()" << std::endl;

    // FIXME:再生キューの破棄
    delete stream_;
  }

  
  void loop(const bool loop) {
    stream_->loop(loop);
  }

  void gain(const float gain) {
    source_.gain(gain);
  }

  void play() {
    // 全てのバッファを再生キューに積む
    for (u_int i = 0; i < BUFFER_NUM; ++i) {
      readStream();
    }
    source_.gain(1.0);
    source_.play();
  }

  void pause(const bool pause) {
    pause_ = pause;
    if (pause) {
      source_.pause();
    }
    else {
      source_.play();
    }
  }
  
  void update() {
    if (pause_) return;
    if (source_.processed() > 0) {
      source_.unqueueBuffer();
      readStream();
    }
  }

  bool isPlaying() {
    return source_.isPlaying();
  }
  
};

}
