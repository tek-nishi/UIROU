
#pragma once

//
// 簡易イージング処理
//

#include "co_easing.hpp"
#include "co_misc.hpp"


namespace ngs {

template <typename T>
class MiniEasing {
  bool exec_;

  EasingType::Type type_;

  float time_;
  float duration_;

  T start_;
  T end_;
  T d_;

  EasingFunc<T> ease_;
  
public:
  MiniEasing() :
    exec_(false),
    ease_(EasingType::Type::LINEAR)
  {}

  MiniEasing(const std::string& type, const float duration, const T& start, const T& end) :
    exec_(true),
    type_(EasingType::fromString(type)),
    time_(0.0f),
    duration_(duration),
    start_(start),
    end_(end),
    d_(end - start),
    ease_(type_)
  {}

  
  // 内部値をDeg→Radに変換
  // TODO:jsonで「これは度です」と書く
  void degToRad() {
    start_ = deg2rad(start_);
    end_   = deg2rad(end_);
    d_     = deg2rad(d_);
  }

  
  // 計算開始
  void start(const EasingType::Type type, const float duration, const T& start, const T& end) {
    exec_     = true;
    type_     = type;
    time_     = 0.0f;
    duration_ = duration;
    start_    = start;
    end_      = end;
    d_        = end - start;

    ease_.type(type_);
  }

  void start(const std::string& type, const float duration, const T& start, const T& end) {
    MiniEasing::start(EasingType::fromString(type), duration, start, end);
  }
  

  // 開始値だけ変更して計算開始
  void start(const T& start) {
    exec_  = true;
    time_  = 0.0f;
    start_ = start;
    d_     = end_ - start;
  }
  
  // 終了値のみ変更して計算開始
  void end(const T& end) {
    exec_ = true;
    time_ = 0.0f;
    end_  = end;
    d_    = end - start_;
  }

  // 計算を中断
  void stop() {
    exec_ = false;
  }

  void resume() {
    exec_ = true;
  }

  // 再計算
  void restart() {
    exec_ = true;
    time_ = 0.0f;
  }
  
  // 長さを変更
  void duration(const float time) {
    duration_ = time;
  }

  float duration() const { return duration_; }
  
  // 開始時の値
  const T& startValue() const {
    return start_;
  }

  // 終了時の値
  const T& endValue() const {
    return end_;
  }
  
  // いっきに最後まで進める
  void toEnd() {
    time_ = duration_;
  }

  // true: 実行中
  bool isExec() const { return exec_; }
  

  // イージング実行
  T operator()(const float delta_time) {
    time_ += delta_time;
    if (time_ >= duration_) {
      time_ = duration_;
      exec_ = false;
    }

    return ease_(time_, start_, d_, duration_);
  }

  // 指定時間の値を求める
  T at(const float time) {
    return ease_((time < duration_) ? time : duration_, start_, d_, duration_);
  }
  
};

}
