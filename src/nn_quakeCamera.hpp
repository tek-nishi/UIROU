
#pragma once

//
// カメラの振動
//

#include <cmath>
#include <list>


namespace ngs {

class QuakeCamera {
  // ミニ振動クラス
  struct Value {
    bool  exec;
    float time;
    float duration;
    float power;
    float speed;
    Vec3f w;
    Vec3f w_st;
  };
  
  std::list<Value> values_;

  // true: 実行中
  bool exec_;

  // 減衰振動定数
  float k_;

  // 振動結果
  Vec3f pos_;


public:
  QuakeCamera() :
    exec_(false),
    k_(4.0f),
    pos_(Vec3f::Zero())
  {
    DOUT << "QuakeCamera()" << std::endl;
  }

  ~QuakeCamera() {
    DOUT << "~QuakeCamera()" << std::endl;
  }

  
  void start(const float power, const float speed) {
    exec_ = true;
    Value value = {
      true,

      0.0f, 1.0f,
      power, speed,

      Vec3f(randomValue(m_pi * speed, m_pi * speed * 1.2f),
            randomValue(m_pi * speed, m_pi * speed * 1.2f),
            randomValue(m_pi * speed, m_pi * speed * 1.2f)),

      Vec3f(randomValue() * m_pi * 2.0f,
            randomValue() * m_pi * 2.0f,
            randomValue() * m_pi * 2.0f)
    };
    values_.push_back(value);
  }

  void stop() {
    exec_ = false;
    values_.clear();
  }
  
  
  void update(const float delta_time) {
    exec_ = !values_.empty();
    if (!exec_) return;

    pos_ = Vec3f::Zero();
    for (auto& value : values_) {
      value.time += delta_time;
      if (value.time >= value.duration) {
        value.time = value.duration;
        value.exec = false;
      }

      // 減衰振動の方程式をそのまま利用
      // y = e^-kx * sin(w0 + wx)
      float exp_value = std::exp(-k_ * value.time);
      for (int i = 0; i < pos_.rows(); ++i) {
        pos_(i) += exp_value * std::sin(value.w_st(i) + value.w(i) * value.time) * value.power;
      }
    }

    // 終了した振動をコンテナから削除
    for (auto it = values_.begin(); it !=values_.end(); /* no count */) {
      if (it->exec) {
        ++it;
        continue;
      }
      // TIPS:listは削除で次のイテレーターを返してくれる
      it = values_.erase(it);
    }
  }

  void setup() const {
    if (!exec_) return;

    translateMatrix(pos_);
  }
  
};

}
