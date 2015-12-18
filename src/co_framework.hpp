
#pragma once

//
// 基本的なアプリの構造を定義
//

#include "co_defines.hpp"
#include <boost/noncopyable.hpp>
#include <picojson.h>
#include "co_os.hpp"
#include "co_keyboard.hpp"
#include "co_mouse.hpp"
#include "co_touch.hpp"
#include "co_audio.hpp"
#include "co_sound.hpp"
#include "co_signal.hpp"
#include "co_view.hpp"
#include "co_time.hpp"
#include "co_glState.hpp"


namespace ngs {

class Framework : private boost::noncopyable {
  Os os_;

  Keyboard keyboard_;
  Mouse    mouse_;
  Touch    touch_;

  Audio audio_;
  std::unique_ptr<Sound> sound_;
  
  Signal signal_;

  View    view_;
  GlState gl_state_;

  Time time_;

  
public:
  Framework() {
    DOUT << "Framework()" << std::endl;
  }

  virtual ~Framework() {
    DOUT << "~Framework()" << std::endl;
  }
  
  const std::string& loadPath() const { return os_.loadPath(); }
  const std::string& savePath() const { return os_.savePath(); }

  Keyboard& keyboard() { return keyboard_; }
  Mouse& mouse() { return mouse_; }
  Touch& touch() { return touch_; }

  Audio& audio() { return audio_; }
  
  void readSound(const std::deque<Sound::Info>& info) {
    sound_ = std::unique_ptr<Sound>(new Sound(info));
  }
  Sound& sound() { return *sound_; }

  Signal& signal() { return signal_; }

  View& view() { return view_; }
  const View& view() const { return view_; }

  GlState& glState() { return gl_state_; }
  const GlState& glState() const { return gl_state_; }

  Time& time() { return time_; }
  const Time& time() const { return time_; }
  
  virtual void update() = 0;
  virtual void draw() = 0;

  virtual void pause() = 0;
  virtual void resume() = 0;
};

}
