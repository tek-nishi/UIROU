
#pragma once

//
// JSONから振動パラメーターを取得
//

#include "co_json.hpp"
#include "co_miniQuake.hpp"
#include "co_misc.hpp"


namespace ngs {

class QuakeParam {
  float duration_;
  float power_;
  float speed_;
  float w_st_;
  float k_;

  
public:
  explicit QuakeParam(const picojson::value& params) :
    duration_(params.at("duration").get<double>()),
    power_(params.at("power").get<double>()),
    speed_(deg2rad<float>(params.at("speed").get<double>())),
    w_st_(deg2rad<float>(params.at("w_st").get<double>())),
    k_(params.at("k").get<double>())
  {}

  
  void start(MiniQuake& quake) const {
    quake.start(duration_, power_, speed_, w_st_, k_);
  }

};

}
