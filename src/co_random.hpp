
#pragma once

//
// 乱数
// 

#include "co_defines.hpp"
#include <random>
#include <boost/noncopyable.hpp>
#include "co_vector.hpp"


namespace ngs {

class Random : private boost::noncopyable {
  std::mt19937 random_;
  std::uniform_real_distribution<float> dist_zero_one_;

public:
  Random() :
    dist_zero_one_(0.0f, 1.0f)
  {}

  void seed(const u_int new_seed = std::mt19937::default_seed) {
    random_.seed(new_seed);
  }
  
  // [0, last)
  int value(const int last) {
    return random_() % last;
  }

  // [0.0f, 1.0f]
  float value() {
    return dist_zero_one_(random_);
  }

  // シングルトン実装
  static Random& instance() {
    static Random instance;
    return instance;
  }
  
};


// シードの設定
void randomSetSeed(const u_int new_seed = std::mt19937::default_seed) {
  Random::instance().seed(new_seed);
}

// [0, last)
int randomValue(const int last) {
  return Random::instance().value(last);
}

// [first, last]
int randomValue(const int first, const int last) {
  return first + Random::instance().value(last - first + 1);
}

// [0.0f, 1.0f]
float randomValue() {
  return Random::instance().value();
}

// [first, last]
float randomValue(const float first, const float last) {
  return first + (last - first) * Random::instance().value();
}

// [param(0), param(1)]
float randomValue(const Vec2f& param) {
  return param(0) + (param(1) - param(0)) * Random::instance().value();
}

}
