
#pragma once

//
// 各種ライブラリのバージョンを表示
//

#include <boost/version.hpp>
#include <assimp/version.h>

namespace ngs {
namespace version {

// OpenGLの初期化が終わった後で呼び出す
void display() {
  DOUT << "Open Asset Importer:" << aiGetVersionMajor() << "." << aiGetVersionMinor() << std::endl;;
  DOUT << "zlib:" << ZLIB_VERSION << std::endl;
  DOUT << "libpng:" << PNG_LIBPNG_VER_STRING << std::endl;
  DOUT << "OpenAL:" << AL_VERSION << std::endl;
  DOUT << "boost:" << BOOST_VERSION << std::endl;
  DOUT << "Eigen:" << EIGEN_WORLD_VERSION << "." << EIGEN_MAJOR_VERSION << "." << EIGEN_MINOR_VERSION << std::endl;
#if !(TARGET_OS_IPHONE)
  DOUT << "GLFW:" << GLFW_VERSION_MAJOR << "." << GLFW_VERSION_MINOR << "." << GLFW_VERSION_REVISION << std::endl;
#endif
  DOUT << "OpenGL Vendor:" << glGetString(GL_VENDOR) << std::endl;
  DOUT << "          GPU:" << glGetString(GL_RENDERER) << std::endl;
  DOUT << "      Version:" << glGetString(GL_VERSION) << std::endl;
}

}
}
