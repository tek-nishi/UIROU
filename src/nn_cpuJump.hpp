
#pragma once

//
// 白UIROUに向かってジャンプしながら近づく
//

#include "co_defines.hpp"
#include "nn_cpu.hpp"


namespace ngs {

class CpuJump : public Cpu {
  Cpu::Target target_;

  Vec2f interval_;
  Vec2f distance_;
  float near_distance_;

  float jump_time_;
  
  
public:
  CpuJump(const picojson::value& params) :
    interval_(vectFromJson<Vec2f>(params.at("interval"))),
    distance_(vectFromJson<Vec2f>(params.at("distance"))),
    near_distance_(params.at("near_distsnce").get<double>()),
    jump_time_(randomValue(interval_))
  {}

  
  void target(const Cpu::Target& target) {
    target_ = target;
  }

  Cpu::Action update(const float delta_time, const Cpu::Self& self) {
    jump_time_ -= delta_time;
    if ((jump_time_ > 0.0f) || !self.targeted) {
      // Jumpできない状況では直進
      Cpu::Action action = {
        1.0f,
        0.0f,
        false, Vec3f::Zero(),
        false
      };
      
      return action;
    }

    // 次のjumpまでの時間
    jump_time_ = randomValue(interval_);
    float dist = distOnCircle(angleFromVecs(self.pos, target_.pos),
                              target_.planet_radius);
    Vec3f pos;
    if (dist < near_distance_) {
      // 一定距離以内は確実に仕留めるX(
      pos = target_.pos;
    }
    else {
      // 一定距離内でjump
      Quatf q(Eigen::AngleAxisf(angleOnCircle(randomValue(distance_), target_.planet_radius),
                                Vec3f::UnitX()));
      q = self.rotate * q;
      pos = q * Vec3f::UnitY();

      // Quatf q(Eigen::AngleAxisf(angle, verticalVector(self.pos)));
      // pos = q * self.pos;
    }

    Cpu::Action action = {
      1.0f,
      0.0f,
      true, pos
    };

    return action;
  }

  // 行動出来ない時間
  void idle(const float delta_time) {
  }

  
private:

  
};

}
