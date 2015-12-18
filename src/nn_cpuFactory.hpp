
#pragma once

//
// 敵CPU生成
//

#include <utility>
#include <unordered_map>
#include "nn_cpuStraight.hpp"
#include "nn_cpuHalf.hpp"
#include "nn_cpuJump.hpp"
#include "nn_cpuZigzag.hpp"
#include "nn_cpuFullspec.hpp"


namespace ngs {

class CpuFactory {
  enum Type {
    JUMP,
    STRAIGHT,
    HALF,
    ZIGZAG,
    FULLSPEC
  };

  std::unordered_map<std::string, Type> tbl_;


public:
  CpuFactory() {
    // FIXME:VS2012で初期化子リストが使えないのでこう書く
    typedef std::pair<std::string, Type> TypeTbl;
    static const TypeTbl tbl[] = {
      std::make_pair("jump", JUMP),
      std::make_pair("straight", STRAIGHT),
      std::make_pair("half", HALF),
      std::make_pair("zigzag", ZIGZAG),
      std::make_pair("fullspec", FULLSPEC)
    };
    
    // あらかじめ用意された配列をコンテナに積む
    for (const auto& obj : tbl) {
      tbl_.insert(std::unordered_map<std::string, Type>::value_type(obj.first, obj.second));
    }
  }

  
  Cpu* operator()(const std::string& type_name, const picojson::value& params) const {
    Type type = tbl_.at(type_name);
    switch (type) {
    case JUMP:
      return new CpuJump(params);

    case STRAIGHT:
      return new CpuStraight(params);

    case HALF:
      return new CpuHalf(params);

    case ZIGZAG:
      return new CpuZigzag(params);

    case FULLSPEC:
      return new CpuFullspec(params);

      
    default:
      assert(0);
      return nullptr;
    }
  }

};

}
