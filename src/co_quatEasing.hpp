
#pragma once

//
// クオータニオン簡易イージング
//

#include "co_easing.hpp"
#include "co_vector.hpp"


namespace ngs {

class QuatEasing {
  bool exec_;

  EasingType::Type type_;

  float time_;
  float duration_;

  Quatf start_;
  Quatf end_;

public:
  QuatEasing() :
    exec_(false)
  {}

  
  // 計算開始
  void start(const EasingType::Type type, const float duration, const Quatf& start, const Quatf& end) {
    exec_     = true;
    type_     = type;
    time_     = 0.0f;
    duration_ = duration;
    start_    = start;
    end_      = end;
  }
  
  void start(const std::string& type, const float duration, const Quatf& start, const Quatf& end) {
    QuatEasing::start(EasingType::fromString(type), duration, start, end);
  }

  // 計算を中断
  void stop() {
    exec_ = false;
  }

  // いっきに最後まで進める
  void toEnd() {
    time_ = duration_;
  }

  void end(const Quatf& end) {
    end_ = end;
  }
  
  // true: 実行中
  bool isExec() const { return exec_; }

  float time() const { return time_; }
  float duration() const { return duration_; }

  // イージング実行
  Quatf operator()(const float delta_time) {
    time_ += delta_time;
    if (time_ >= duration_) {
      time_ = duration_;
      exec_ = false;
    }

    EasingFunc<float> easing(type_);
    float t = easing(time_, 0.0f, 1.0f, duration_);
    Quatf current = start_.slerp(t, end_);

    return current;
  }
  
};

}
