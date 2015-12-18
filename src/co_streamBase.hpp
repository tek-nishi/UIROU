
#pragma once

//
// ストリーミング基底クラス
//

#include <boost/noncopyable.hpp>


namespace ngs {

struct StreamBase : private boost::noncopyable {
  virtual ~StreamBase() {}
  
  virtual bool isStereo() const = 0;
  virtual u_int sampleRate() const = 0;
  virtual void loop(const bool loop) = 0;
  virtual void toTop() = 0;
  virtual bool isEnd() const = 0;
  virtual size_t read(std::vector<char>& buffer) = 0;
};

}
