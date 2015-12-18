
#pragma once

//
// CPU基本定義
//

#include "co_defines.hpp"


namespace ngs {

struct Cpu {

  struct Target {
    // 目標の位置ベクトル
    Vec3f pos;
    // 目標の向きベクトル
    Vec3f angle;
    // 目標の回転クオータニオン
    Quatf rotate;

    // 惑星の半径
    float planet_radius;
  };

  struct Self {
    // 自分の位置ベクトル
    Vec3f pos;
    // 自分の向きベクトル
    Vec3f angle;
    // 自分の回転クオータニオン
    Quatf rotate;

    // ターゲット発見
    bool targeted;
    
    // 自分の残り体力[0.0, 1.0]
    float hp_rate;
  };

  struct Action {
    // アクセル開度
    float acc;
    // 旋回
    float yaw;

    // ジャンプ
    bool  jump;
    Vec3f jump_pos;

    // ターゲット変更
    bool re_target;
  };

  virtual ~Cpu() {
    DOUT << "~Cpu()" << std::endl;
  }

  virtual void target(const Cpu::Target& target) = 0;

  virtual Cpu::Action update(const float delta_time, const Cpu::Self& self) = 0;
  virtual void idle(const float delta_time) = 0;
  
};
  
}
