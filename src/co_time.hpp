
#pragma once

//
// 時間管理
//


#if (TARGET_OS_IPHONE)

// 無名名前空間を使う
namespace {

#include <sys/time.h>

// GLFWと同じ役割の関数を定義
double glfwGetTime() {
  timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

}

#endif

namespace ngs {

class Time {
  double start_;

public:
  Time() :
    start_(glfwGetTime())
  {}


  double current() const {
    return glfwGetTime() - start_;
  }
  
};

}
