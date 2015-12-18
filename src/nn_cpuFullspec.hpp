
#pragma once

//
// 強敵
//

#include "co_defines.hpp"
#include "nn_cpu.hpp"


namespace ngs {

class CpuFullspec : public Cpu {
  Cpu::Target target_;
  
  Vec2f distance_;
  Vec2f acc_;
  Vec2f interval_;
  Vec2f jump_distance_;
  float near_distance_;
  Vec2f target_interval_;

  float jump_time_;
  float target_time_;

  
public:
  CpuFullspec(const picojson::value& params) :
    distance_(vectFromJson<Vec2f>(params.at("distance"))),
    acc_(vectFromJson<Vec2f>(params.at("acc"))),
    interval_(vectFromJson<Vec2f>(params.at("interval"))),
    jump_distance_(vectFromJson<Vec2f>(params.at("jump_distance"))),
    near_distance_(params.at("near_distsnce").get<double>()),
    target_interval_(vectFromJson<Vec2f>(params.at("target_interval"))),
    jump_time_(randomValue(interval_)),
    target_time_(randomValue(target_interval_))
  {}
    
  
  void target(const Cpu::Target& target) {
    target_ = target;
  }

  Cpu::Action update(const float delta_time, const Cpu::Self& self) {
    bool re_target = false;
    if (target_time_ > 0.0f) {
      target_time_ -= delta_time;
      if (target_time_ <= 0.0f) {
        re_target = true;
        target_time_ = randomValue(target_interval_);
      }
    }
    
    jump_time_ -= delta_time;
    if (jump_time_ > 0.0f) {
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
        re_target
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
      Quatf q(Eigen::AngleAxisf(angleOnCircle(randomValue(jump_distance_), target_.planet_radius),
                                Vec3f::UnitX()));
      q = self.rotate * q;
      pos = q * Vec3f::UnitY();
    }

    Cpu::Action action = {
      1.0f,
      0.0f,
      true, pos,
      re_target
    };
    
    return action;
  }
  
  // 行動出来ない時間
  void idle(const float delta_time) {
  }
  
private:

  
};

}
