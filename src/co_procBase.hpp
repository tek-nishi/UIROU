
#pragma once

//
// Appから実行されるプログラム
//

#include "co_defines.hpp"
#include <boost/noncopyable.hpp>


namespace ngs {

struct ProcBase : private boost::noncopyable {
  virtual ~ProcBase() {}

  virtual void update(const float delta_time) = 0;
  virtual void draw() = 0;
  
  virtual void pause() = 0;
  virtual void resume() = 0;
};

}
