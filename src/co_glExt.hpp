
#pragma once

// 
// OpenGL拡張機能
//


namespace ngs {

#if defined (_MSC_VER)

// 初期化
// false: 拡張機能は使えない
bool initGlExt() {
  GLenum result_code = glewInit();
  return result_code == GLEW_OK;
}

#else

// OSX, iOSでは必ず使える
bool initGlExt() { return true; }

#endif

}
