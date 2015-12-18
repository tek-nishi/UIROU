
#pragma once

//
// テクスチャ管理
//

#include <string>
#include <boost/noncopyable.hpp>
#include "co_png.hpp"
#include "co_fileUtil.hpp"
#include "co_misc.hpp"


namespace ngs {

class Texture : public boost::noncopyable {
	GLuint id_;
  int width_;
  int height_;
	std::string name_;
	bool mipmap_;


  // テクスチャの基本的なパラメーター設定を行う
  static void setupTextureParam(const bool mipmap) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST);
  }

  
	void setupPng(const std::string& filename, const bool mipmap) {
		Png png_obj(filename);
    width_ = png_obj.width();
    height_ = png_obj.height();
    if ((width_ != int2pow(width_)) || (height_ != int2pow(height_))) {
      DOUT << "Texture size error " << width_ << ":" << height_ << std::endl;
      // サイズが2のべき乗でなければエラー
      return;
    }

		glBindTexture(GL_TEXTURE_2D, id_);
		setupTextureParam(mipmap);

		GLint type = (png_obj.type() == PNG_COLOR_TYPE_RGB) ? GL_RGB : GL_RGBA;
#if defined (_MSC_VER)
		if (mipmap) {
			gluBuild2DMipmaps(GL_TEXTURE_2D, type, width_, height_, type, GL_UNSIGNED_BYTE, png_obj.image());
			// FIXME:WindowsだとglGenerateMipmap()が使えない？
		}
		else {
			glTexImage2D(GL_TEXTURE_2D, 0, type, width_, height_, 0, type, GL_UNSIGNED_BYTE, png_obj.image());
		}
#else
		glTexImage2D(GL_TEXTURE_2D, 0, type, width_, height_, 0, type, GL_UNSIGNED_BYTE, png_obj.image());
		if (mipmap) glGenerateMipmap(GL_TEXTURE_2D);
#endif
		
    DOUT << "Texture:" << name_ << ((type == PNG_COLOR_TYPE_RGB) ? " RGB" : " RGBA") << std::endl;
	}
	
public:
	Texture(const std::string& filename, const bool mipmap = false) :
		name_(getFileName(filename)),
		mipmap_(mipmap)
	{
    DOUT << "Texture()" << std::endl;
		glGenTextures(1, &id_);
    setupPng(filename, mipmap);
	}
	
	~Texture() {
    DOUT << "~Texture()" << std::endl;
		glDeleteTextures(1, &id_);
	}

  int width() const { return width_; }
  int height() const { return height_; }
	const std::string& name() const { return name_; }

	void bind() const {
		glBindTexture(GL_TEXTURE_2D, id_);
	}

	void unbind() const {
		glBindTexture(GL_TEXTURE_2D, 0);
	}
  
};

}
