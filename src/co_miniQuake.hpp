
#pragma once

//
// 簡易振動生成クラス
//


namespace ngs {

class MiniQuake {
  bool  exec_;
  float time_;
  float duration_;
  float power_;
  float speed_;
  float w_st_;
  float k_;
  

public:
  MiniQuake() :
    exec_(false)
  {}

  // 振動開始
  void start(const float duration, const float power, const float speed, const float w_st, const float k) {
    exec_     = true;
    time_     = 0.0f;
    duration_ = duration;
    power_    = power;
    w_st_     = w_st;
    speed_    = speed;
    k_        = k;
  }

  void stop() {
    exec_ = false;
  }

  // 計算中か返す
  bool isExec() const {
    return exec_;
  }


  // 振動の計算
  float operator()(const float delta_time) {
    // 実行中でなければ即時終了
    if (!exec_) return 0.0f;
    
    time_ += delta_time;
    if (time_ >= duration_) {
      time_ = duration_;
      exec_ = false;
    }

    // 減衰振動の方程式をそのまま利用
    // y = e^-kx * sin(w0 + wx)
    float exp_value = std::exp(-k_ * time_);
    return exp_value * std::sin(w_st_ + speed_ * time_) * power_;
  }
  
};

}
