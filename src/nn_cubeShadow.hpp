
#pragma once

//
// 影オブジェクト
//

#include "nn_vbo.hpp"


namespace ngs {

class CubeShadow {
  // モデルデータを読み込む時の設定
  enum {
    import_flags = aiProcess_Triangulate |
                   aiProcess_JoinIdenticalVertices |
                   aiProcess_FlipUVs |
                   aiProcess_SortByPType |
                   aiProcess_OptimizeMeshes
  };

  // 頂点データ定義
  struct Vtx {
    float x, y, z;
  };
  struct Uv {
    float u, v;
  };
  struct Body {
    Vtx vtx;
    Uv uv;
  };
  std::deque<Body> body_;

  // 面データ定義
	struct Face {
		GLushort v1, v2, v3;
	};
  // 描画頂点数
  GLuint points_;

  // VBO
  Vbo vtx_vbo_;
  std::shared_ptr<Vbo> face_vbo_;

  GrpCol color_;

  std::shared_ptr<Texture> texture_;
  std::shared_ptr<EasyShader> shader_;
  
  
public:
  CubeShadow(const std::string& model_file, const std::string& texture_file,
             ShaderHolder& shader_holder) :
    face_vbo_(std::make_shared<Vbo>()),
    color_(0.0f, 0.0f, 0.0f, 1.0f),
    texture_(std::make_shared<Texture>(texture_file)),
    shader_(shader_holder.read("shadow"))
  {
    DOUT << "CubeShadow()" << std::endl;

    // Open Asset Importerを利用してモデルデータを読み込む
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(model_file, import_flags);
    if (!scene) {
      DOUT << importer.GetErrorString() << std::endl;
    }
    DOUT << "Mesh:" << scene->mNumMeshes << std::endl;

    std::vector<Face> faces;

    // メッシュ生成
    for (u_int i = 0; i < scene->mNumMeshes; ++i) {
      const aiMesh& mesh = *(scene->mMeshes[i]);

      // 先に面データ生成
      const GLushort face_index = static_cast<GLushort>(body_.size());
      aiFace* f = mesh.mFaces;
      for (u_int fi = 0; fi < mesh.mNumFaces; ++fi) {
        // 三角ポリゴン以外はエラー
        assert(f->mNumIndices == 3);

        Face face;
        face.v1 = f->mIndices[0] + face_index;
        face.v2 = f->mIndices[1] + face_index;
        face.v3 = f->mIndices[2] + face_index;
        faces.push_back(face);
        
        ++f;
      }
      
      // 頂点データ生成
      const aiVector3D* v  = mesh.mVertices;
      const aiVector3D* uv = mesh.mTextureCoords[0];
      for (u_int vi = 0; vi < mesh.mNumVertices; ++vi) {
        Body body = {
          { v->x, v->y, v->z },
          { uv->x, uv->y }
        };
        body_.push_back(body);

        ++v;
        ++uv;
      }
    }
    
    // 描画総頂点数
    points_ = static_cast<GLuint>(faces.size() * 3);

    // 面データは転送しておく
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face_vbo_->handle());
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Face) * faces.size(), &faces[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  ~CubeShadow() {
    DOUT << "~CubeShadow()" << std::endl;
  }


  // 影の半径と惑星の半径から最終的なデータを生成
  void setup(const float radius, const float planet_radius, const float gap) const {
    // VBOに転送する頂点データ(あらかじめ容量を割り当てておく)
    std::vector<Body> body;
    body.reserve(body_.size());

    // 影の半径と惑星の半径から各頂点を再計算する
    for (auto b : body_) {
      b.vtx.x *= radius;
      b.vtx.z *= radius;
      float len = std::sqrt(b.vtx.x * b.vtx.x + b.vtx.z * b.vtx.z);
      b.vtx.y = std::sqrt(planet_radius * planet_radius - len * len) - planet_radius + gap;
      body.push_back(b);
    }

    // VBOに転送
		glBindBuffer(GL_ARRAY_BUFFER, vtx_vbo_.handle());
		glBufferData(GL_ARRAY_BUFFER, sizeof(Body) * body.size(), &body[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  void color(const GrpCol& col) { color_ = col; }


  void draw(Framework& fw, const Mat4f& matrix) const {
    const EasyShader& shader = *shader_;

    // TIPS:iOSのsnapshotの不具合っぽいのがあるので、
    //      ブレンディングを「引き算」にしておく
    fw.glState().blend(true);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
    glDepthMask(GL_FALSE);
    
    shader();

    pushMatrix();
    multMatrix(matrix);
    
    // glUniformMatrix4fv(shader.uniform("projectionMatrix"), 1, GL_FALSE, getProjectionMatrix().data());
    glUniformMatrix4fv(shader.uniform("modelViewMatrix"), 1, GL_FALSE, getModelMatrix().data());

    glUniform4f(shader.uniform("material_diffuse"), color_(0), color_(1), color_(2), color_(3));
    
    glBindBuffer(GL_ARRAY_BUFFER, vtx_vbo_.handle());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face_vbo_->handle());
    
    GLint position = shader.attrib("position");
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, sizeof(Body), 0);

    GLint uv = shader.attrib("uv");
    glEnableVertexAttribArray(uv);
    glVertexAttribPointer(uv, 2, GL_FLOAT, GL_FALSE, sizeof(Body), (GLvoid *)offsetof(Body, uv));

    glUniform1i(shader.uniform("sampler"), 0);
    texture_->bind();
    
    glDrawElements(GL_TRIANGLES, points_, GL_UNSIGNED_SHORT, 0);

    glDisableVertexAttribArray(position);
    glDisableVertexAttribArray(uv);
    texture_->unbind();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    popMatrix();

    fw.glState().blend(false);
    glDepthMask(GL_TRUE);
  }
  
};

}
