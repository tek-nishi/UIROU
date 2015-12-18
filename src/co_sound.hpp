
#pragma once

//
// 効果音管理
// wav と ogg vorbis を自動判定して読み込み
//

#include <string>
#include <deque>
#include <memory>
#include <map>
#include <cassert>
#include <boost/noncopyable.hpp>
#include "co_audio.hpp"
#include "co_wav.hpp"
#include "co_ogg.hpp"
#include "co_vector.hpp"


namespace ngs {

class Sound : private boost::noncopyable {
public:
  struct Info {
    std::string name;
    std::string file;
    std::string category;
    float gain;
    bool loop;
  };
  
private:
  struct Buffer {
    Audio::Buffer buffer;
    std::string category;
    float gain;
    bool loop;
  };
  
  std::map<std::string, std::unique_ptr<Buffer> > buffer_;
  std::map<std::string, std::unique_ptr<Audio::Source> > source_;
  
public:
  explicit Sound(const std::deque<Info>& info) {
    DOUT << "Sound()" << std::endl;
    
    for (const auto& it : info) {
      if (buffer_.find(it.name) != buffer_.end()) assert(!"sound name is already exists.");

      Buffer* ptr = new Buffer();
      std::unique_ptr<Buffer> p(ptr);
      p->category = it.category;
      p->gain = it.gain;
      p->loop = it.loop;

      // ogg vorvisかwavかを判定して読み込む
      if (Ogg::isOgg(it.file)) {
        Ogg data(it.file);
        p->buffer.bind(data.isStereo(), data.data(), data.size(), data.sampleRate());
      }
      else {
        Wav data(it.file);
        p->buffer.bind(data.isStereo(), data.data(), data.size(), data.sampleRate());
      }

      buffer_[it.name] = std::move(p);
      // unique_ptrをコンテナに格納するにはムーブを使う

      if (source_.find(it.category) == source_.end()) {
        source_[it.category] = std::unique_ptr<Audio::Source>(new Audio::Source);
        // 存在しないカテゴリなら、そのカテゴリ名の再生バッファを作成
      }
    }
  }

  ~Sound() {
    DOUT << "~Sound()" << std::endl;
  }

  
  void play(const std::string& name, const Vec3f& pos = Vec3f::Zero()) {
    const auto itb = findBuffer(buffer_, name);
    const auto its = findBuffer(source_, itb->second->category);

    its->second->position(pos);
    its->second->play(itb->second->buffer, itb->second->gain, itb->second->loop);
    // FIXME:ちょっと長いよね…
  }

  
  void stop(const std::string& category) {
    const auto its = findBuffer(source_, category);
    its->second->stop();
  }

  void gain(const std::string& category, const float volume) {
    const auto its = findBuffer(source_, category);
    its->second->gain(volume);
  }


private:
  template <typename T>
  static typename T::const_iterator findBuffer(const T& buffer, const std::string& name) {
    auto it = buffer.find(name);
    if (it == buffer.cend()) {
      DOUT << "no buffer:" << name << std::endl;
      throw;
    }
    return it;
  }
  
};

}
