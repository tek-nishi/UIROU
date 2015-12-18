
#pragma once

//
// ジグザグ走行
// TODO:「白UIROUが視界に入ったら直進」
//

#include "co_defines.hpp"
#include "nn_cpu.hpp"


namespace ngs {

class CpuZigzag : public Cpu {
  Cpu::Target target_;

  Vec2f straight_interval_;
  Vec2f straight_acc_;
  Vec2f rotate_interval_;
  Vec2f rotate_acc_;
  
  enum Mode {
    STRAIGHT,
    ROTATE
  };
  Mode mode_;
  float action_time_;
  float rotate_;
  float acc_;
  
  
public:
  CpuZigzag(const picojson::value& params) :
    straight_interval_(vectFromJson<Vec2f>(params.at("straight_interval"))),
    straight_acc_(vectFromJson<Vec2f>(params.at("straight_acc"))),
    rotate_interval_(vectFromJson<Vec2f>(params.at("rotate_interval"))),
    rotate_acc_(vectFromJson<Vec2f>(params.at("rotate_acc"))),
    mode_(STRAIGHT),
    action_time_(randomValue(straight_interval_)),
    acc_(randomValue(straight_acc_))
  {}

  
  void target(const Cpu::Target& target) {
    target_ = target;
  }

  Cpu::Action update(const float delta_time, const Cpu::Self& self) {
    switch (mode_) {
    case STRAIGHT:
      {
        Cpu::Action action = {
          acc_,
          0.0f,
          false, Vec3f::Zero(),
          false
        };
        
        action_time_ -= delta_time;
        if (action_time_ <= 0.0f) {
          mode_        = ROTATE;
          action_time_ = randomValue(rotate_interval_);
          rotate_      = (randomValue(100) < 50) ? m_pi : -m_pi;
          acc_         = randomValue(rotate_acc_);
        }

        return action;
      }

    case ROTATE:
      {
        Cpu::Action action = {
          acc_,
          rotate_,
          false, Vec3f::Zero(),
          false
        };
        
        action_time_ -= delta_time;
        if (action_time_ <= 0.0f) {
          mode_        = STRAIGHT;
          action_time_ = randomValue(straight_interval_);
          acc_         = randomValue(straight_acc_);
        }

        return action;
      }

    default:
      {
        assert(0);
        Cpu::Action action = {
          1.0f,
          0.0f,
          false, Vec3f::Zero(),
          false
        };
        return action;
      }
    }
  }
  
  // 行動出来ない時間
  void idle(const float delta_time) {
  }
  
private:

  
};

}
