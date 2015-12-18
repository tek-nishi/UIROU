
#pragma once

// 
// 宣伝ダイアログ
//


namespace ngs {
namespace advertisement {

#if (TARGET_OS_IPHONE)
  
// 初期化
void init(ViewController* view_controller);
// ダイアログ表示
void popup();
// 再初期化
void reset();

#else

void popup() {}
void reset() {}

#endif

}
}
