
#pragma once

//
// In と Out を持つ簡易Easing
//

#include "co_miniEasing.hpp"
#include "co_json.hpp"

namespace ngs {

template <typename T>
class EaseInOut {
  MiniEasing<T> ease_in_;
  MiniEasing<T> ease_out_;

  T value_;
  
  float duration_;
  float time_;

  bool exec_;


public:
  EaseInOut(const picojson::value& in_value, const picojson::value& out_value, const float duration) :
    ease_in_(miniEasingFromJson<T>(in_value)),
    ease_out_(miniEasingFromJson<T>(out_value)),
    value_(ease_in_.startValue()),
    duration_(duration),
    exec_(false)
  {}


  bool isExec() const { return exec_; }
  
  void start() {
    ease_in_.restart();
    ease_out_.stop();
    time_ = 0.0f;
    exec_ = true;
  }

  void stop() {
    exec_ = false;
  }

  void inStart(const T& value) {
    ease_in_.start(value);
  }
  void inEnd(const T& value) {
    ease_in_.end(value);
  }
  
  void outStart(const T& value) {
    ease_out_.start(value);
  }
  void outEnd(const T& value) {
    ease_out_.end(value);
  }

  void duration(const float duration) {
    duration_ = duration;
  }

  // 強制的に終了演出へ移行
  void doEnd() {
    if (exec_ && !ease_out_.isExec()) {
      ease_in_.stop();
      ease_out_.restart();
    }
  }
  

  T operator()(const float delta_time) {
    if (!exec_) return value_;
    
    if (ease_in_.isExec()) {
      value_ = ease_in_(delta_time);
      return value_;
    }
    else if (ease_out_.isExec()) {
      value_ = ease_out_(delta_time);
      exec_ = ease_out_.isExec();
      return value_;
    }

    if (time_ < duration_) {
      time_ += delta_time;
      if (time_ >= duration_) {
        ease_out_.restart();
      }
    }
    return value_;
  }
  
};

}
