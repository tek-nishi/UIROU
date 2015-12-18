
#pragma once

//
// モデルのマテリアルを定義
// ※色、テクスチャ
//

#include <assimp/scene.h>
#include "co_vector.hpp"
#include "co_texMng.hpp"


namespace ngs {

class Material {
	Vec3f diffuse_;
	Vec3f specular_;
  float shininess_;
	Vec3f emissive_;

  bool has_texture_;
  TexMng::TexPtr texture_;
  bool wrap_u_;
  bool wrap_v_;

  
public:
  explicit Material(const aiMaterial& material, TexMng& tex_mng, const std::string& path) :
    diffuse_(Vec3f::Zero()),
    specular_(Vec3f::Zero()),
    shininess_(0.0),
    emissive_(Vec3f::Zero()),
    has_texture_(false),
    wrap_u_(false),
    wrap_v_(false)
  {
    DOUT << "Material()" << std::endl;

    {
      // 拡散光
      aiColor3D color(0.0f, 0.0f, 0.0f);
      if (material.Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
        diffuse_ << color.r, color.g, color.b;

        DOUT << "diffuse:" <<
          diffuse_.x() << "," << 
          diffuse_.y() << "," <<
          diffuse_.z() <<
          std::endl;
      }
    }
    
    {
      // スペキュラー
      aiColor3D color(0.0f, 0.0f, 0.0f);
      if (material.Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
        specular_ << color.r, color.g, color.b;
        
        DOUT << "specular:" <<
          specular_.x() << "," <<
          specular_.y() << "," <<
          specular_.z() <<
          std::endl;
      }
    }
    
    {
      // スペキュラー拡散率
      float shininess = 0.0f;
      if (material.Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
        shininess_ = shininess;
        DOUT << "shininess:" << shininess_ << std::endl;
      }
    }
    
    {
      // エミッシブ(自己発光)
      aiColor3D color(0.0f, 0.0f, 0.0f);
      if (material.Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS) {
        emissive_ << color.r, color.g, color.b;
        
        DOUT << "emissive:" <<
          emissive_.x() << "," <<
          emissive_.y() << "," <<
          emissive_.z() <<
          std::endl;
      }
    }

    {
      // テクスチャ
      aiString name;
      if (material.Get(AI_MATKEY_TEXTURE_DIFFUSE(0), name) == AI_SUCCESS) {
        // パスを除いた名前を生成
        std::string texture_file = getFileNameFull(std::string(name.C_Str()));
        // テクスチャを読み込む
        texture_ = tex_mng.read(path + texture_file);
        has_texture_ = true;
      }
    }

#if 0
    // FIXME:Colladaが対応してない感じ
    {
      int value;
      if (material.Get(AI_MATKEY_MAPPINGMODE_U_DIFFUSE(0), value) == AI_SUCCESS) {
        DOUT << "AI_MATKEY_MAPPINGMODE_U_DIFFUSE:" << value << std::endl;
        if (value) wrap_u_ = true;
      }
    }
    {
      int value;
      if (material.Get(AI_MATKEY_MAPPINGMODE_V_DIFFUSE(0), value) == AI_SUCCESS) {
        DOUT << "AI_MATKEY_MAPPINGMODE_V_DIFFUSE:" << value << std::endl;
        if (value) wrap_v_ = true;
      }
    }
#endif
  }
  
  ~Material() {
    DOUT << "~Material()" << std::endl;
  }

  const Vec3f& diffuse() const { return diffuse_; }
  void diffuse(const Vec3f& diffuse) { diffuse_ = diffuse; }
  
	const Vec3f& specular() const { return specular_; }
  void specular(const Vec3f& specular) { specular_ = specular; }
  
  float shininess() const { return shininess_; }
  void shininess(const float shininess) { shininess_ = shininess; }

  const Vec3f& emissive() const { return emissive_; }
  void emissive(const Vec3f& emissive) { emissive_ = emissive; }

  bool texture() const { return has_texture_; }

  void texture(TexMng::TexPtr texture) {
    assert(has_texture_);
    
    texture_ = texture;
  }

  void bindTexture() const {
    texture_->bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_u_ ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_v_ ? GL_REPEAT : GL_CLAMP_TO_EDGE);
  }

  bool textureWrapU() const { return wrap_u_; }
  bool textureWrapV() const { return wrap_v_; }
  void textureWrapU(const bool wrap) { wrap_u_ = wrap; }
  void textureWrapV(const bool wrap) { wrap_v_ = wrap; }
  
};

}
