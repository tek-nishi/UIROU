
#pragma once

// 
// 評価ダイアログ
//


namespace ngs {
namespace rating {

#if (TARGET_OS_IPHONE)
  
// 初期化
void init(ViewController* view_controller);
// ダイアログ表示
void popup();
// 元に戻す
void reset();

#else

void popup() {}
void reset() {}

#endif

}
}
