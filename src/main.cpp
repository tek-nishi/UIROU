//
// メインプログラム
//

#include "co_defines.hpp"
#include <cstdlib>
#include "co_execGLFW.hpp"


int main() {
  using namespace ngs;
  
  // 初期化に失敗したら即終了
  if (!exec::initialize()) return EXIT_FAILURE;

  // アプリが有効な間はずっと更新と描画を行う
  while (exec::isActive()) {
    exec::update();
    exec::draw();
  }

  // アプリを破棄して終了
  exec::destroy();
}
