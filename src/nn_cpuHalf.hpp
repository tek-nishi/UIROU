
#pragma once

//
// HPが半減したらパターンが変わる
//

#include "co_defines.hpp"
#include "nn_cpu.hpp"


namespace ngs {

class CpuHalf : public Cpu {
  Cpu::Target target_;

  Vec2f acc_;
  
  
public:
  CpuHalf(const picojson::value& params) :
    acc_(vectFromJson<Vec2f>(params.at("acc")))
  {}

  
  void target(const Cpu::Target& target) {
    target_ = target;
  }

  Cpu::Action update(const float delta_time, const Cpu::Self& self) {
    float yaw = 0.0f;
    if (self.targeted) {
      // 自分の位置と目標の位置の外積と、移動ベクトルが一致すれば
      // 目標へ到達できる
      yaw = angleFromVecs(self.angle, self.pos.cross(target_.pos));

      // 目標の位置と移動ベクトルの内積で右回りか左回りか判別できる
      if (self.angle.dot(target_.pos) < 0.0f) yaw = -yaw;
    }

    Cpu::Action action = {
      self.hp_rate > 0.5f ? acc_(0) : acc_(1),
      yaw,
      false, Vec3f::Zero(),
      false
    };

    return action;
  }
  
  // 行動出来ない時間
  void idle(const float delta_time) {
  }
  
private:

  
};

}
