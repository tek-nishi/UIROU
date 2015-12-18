
#pragma once

//
// 白UIROUに向かって直進する
//

#include "co_defines.hpp"
#include "nn_cpu.hpp"


namespace ngs {

class CpuStraight : public Cpu {
  Cpu::Target target_;
  
  Vec2f distance_;
  Vec2f acc_;

  
public:
  CpuStraight(const picojson::value& params) :
    distance_(vectFromJson<Vec2f>(params.at("distance"))),
    acc_(vectFromJson<Vec2f>(params.at("acc")))
  {}
    
  
  void target(const Cpu::Target& target) {
    target_ = target;
  }

  Cpu::Action update(const float delta_time, const Cpu::Self& self) {
    float yaw = 0.0f;
    float acc = acc_(1);
    if (self.targeted) {
      // 自分の位置と目標の位置の外積と、移動ベクトルが一致すれば
      // 目標へ到達できる
      yaw = angleFromVecs(self.angle, self.pos.cross(target_.pos));

      // 目標の位置と移動ベクトルの内積で右回りか左回りか判別できる
      if (self.angle.dot(target_.pos) < 0.0f) yaw = -yaw;

      // 目標までの距離でアクセルを決める
      // distsnce_(0)でacc_(0)、distance_(1)でacc_(1)
      float dist = distOnCircle(angleFromVecs(target_.pos, self.pos), target_.planet_radius);
      EasingFunc<float> ease(EasingType::Type::LINEAR);
      acc = ease(minmax(dist, distance_(0), distance_(1)) - distance_(0),
                 acc_(0), acc_(1) - acc_(0),
                 distance_(1) - distance_(0));
    }

    Cpu::Action action = {
      acc,
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
