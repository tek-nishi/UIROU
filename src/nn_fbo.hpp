
#pragma once

//
// オフスクリーンバッファ
// サイズは２のべき乗であること
//

namespace ngs {

#include "co_defines.hpp"
#include <boost/noncopyable.hpp>
#include "co_vector.hpp"


class Fbo : private boost::noncopyable {
  Vec2i size_;
  GLuint texture_id_;
  GLuint depth_id_;
  
  GLuint framebuffer_id_;

public:
  explicit Fbo(const Vec2i& size) :
    size_(size)
  {
    DOUT << "FBO:" << size.x() << "x" << size.y() << std::endl;

    // オフスクリーンレンダリング用のテクスチャを生成
    glGenTextures(1, &texture_id_);
    glBindTexture(GL_TEXTURE_2D, texture_id_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x(), size.y(), 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 初期化が終わったら拘束を解除
    glBindTexture(GL_TEXTURE_2D, 0);

    // オフスクリーンレンダリング用の深度バッファを生成
    glGenRenderbuffers(1, &depth_id_);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_id_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, size.x(), size.y());

    // 初期化が終わったら拘束を解除
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // 現在のフレームバッファのidを退避
    GLint current_fbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current_fbo);

    // オフスクリーンレンダリング用にフレームバッファを生成
    glGenFramebuffers(1, &framebuffer_id_);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id_);

    // フレームバッファにオフスクリーンレンダリング用のテクスチャを結合
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id_, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_id_);
    
    // check FBO status
    GLenum fbo_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fbo_status != GL_FRAMEBUFFER_COMPLETE) {
      DOUT << "FBO not complete:" <<  fbo_status << std::endl;
    }
    
    // 元のフレームバッファに戻す
    glBindFramebuffer(GL_FRAMEBUFFER, current_fbo);
  }

  ~Fbo() {
    // 各種リソースを破棄
    glDeleteFramebuffers(1, &framebuffer_id_);
    glDeleteTextures(1, &texture_id_);
    glDeleteRenderbuffers(1, &depth_id_);
    
  }


  GLint bind() const {
    GLint current_fbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current_fbo);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id_);	
    glViewport(0, 0, size_.x(), size_.y());

    return current_fbo;
  }

  void unbind(const GLint framebuffer_id) const {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
  }

  void bindTexture() const {
    glBindTexture(GL_TEXTURE_2D, texture_id_);
  }
  
  
private:
  

  
};

}
