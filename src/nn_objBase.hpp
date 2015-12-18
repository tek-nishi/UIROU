
#pragma once

//
// ゲーム内オブジェクトの基底クラス
//

#include "co_defines.hpp"
#include "co_signal.hpp"


namespace ngs {

struct ObjBase {
  virtual ~ObjBase() {}

  // 有効判定
  virtual bool isActive() const = 0;
  // メッセージ処理
  virtual void message(const int msg, Signal::Params& arguments) = 0;
};

}
