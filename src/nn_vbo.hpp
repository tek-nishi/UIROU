
#pragma once

//
// VBO
// TIPS:shared_ptrで扱う為の定義
//

#include <boost/noncopyable.hpp>


namespace ngs {

class Vbo {
  GLuint handle_;

public:
  Vbo() {
    glGenBuffers(1, &handle_);
  }
  
  ~Vbo() {
		glDeleteBuffers(1, &handle_);
  }

  // TIPS:自分でコピーする
  Vbo(const Vbo& rhs) {
    DOUT << "Vbo(const Vbo& rhs)" << std::endl;

    glGenBuffers(1, &handle_);
  }

  
  GLuint handle() const { return handle_; }

};

}
