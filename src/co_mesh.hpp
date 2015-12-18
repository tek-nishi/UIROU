
#pragma once

//
// モデルのメッシュを定義
// ※頂点、法線、テクスチャ座標
// FIXME:OpenGL依存
//

#include <vector>
#include <cassert>
#include <boost/noncopyable.hpp>
#include <assimp/scene.h>


namespace ngs {

class Mesh : public boost::noncopyable {
public:
  enum Buffer {
    ARRAY,
    ELEMENT_ARRAY,
    // 確保するVBOの数
    NUM_VBO
  };
  
	struct Vtx {
		GLfloat x, y, z;
	};
	struct Uv {
		GLfloat u, v;
	};
	struct Body {
		Vtx vertex;
		Vtx normal;
		Uv uv;
	};
	struct Face {
		GLushort v1, v2, v3;
	};

private:
	GLuint vbo_[NUM_VBO];

  bool has_normal_;
  bool has_texture_;
  u_int faces_;
	GLuint points_;
  u_int material_index_;

  Vec3f min_pos_;
  Vec3f max_pos_;
  
public:
  explicit Mesh(const aiMesh& mesh) :
    has_normal_(mesh.HasNormals()),
    has_texture_(mesh.HasTextureCoords(0)),
    faces_(mesh.mNumFaces),
    points_(mesh.mNumFaces * 3),
    material_index_(mesh.mMaterialIndex),
    min_pos_(FLT_MAX, FLT_MAX, FLT_MAX),
    max_pos_(-FLT_MAX, -FLT_MAX, -FLT_MAX)
  {
    DOUT << "Mesh()" << std::endl;
#if 0
    DOUT << "Mesh Vertices:" << mesh.mNumVertices << std::endl;
    DOUT << "Mesh Normals:" << mesh.HasNormals() << std::endl;
    DOUT << "Mesh UV:" << mesh.HasTextureCoords(0) << std::endl;
    DOUT << "Mesh VtertexColors:" << mesh.HasVertexColors(0) << std::endl;
    DOUT << "Mesh Faces:" << mesh.mNumFaces << std::endl;
    DOUT << "Mesh Material:" << mesh.mMaterialIndex << std::endl;
#endif

    // 頂点情報を生成
    const aiVector3D* v = mesh.mVertices;
    const aiVector3D* n = mesh.mNormals;
    const aiVector3D* uv = mesh.mTextureCoords[0];

    std::vector<Body> body;
    body.reserve(mesh.mNumVertices);
    // TIPS:あらかじめvectorのサイズを予約し、push_backによるコピーを防ぐ

    for (u_int i = 0; i < mesh.mNumVertices; ++i) {
      Body obj;
      obj.vertex.x = v->x;
      obj.vertex.y = v->y;
      obj.vertex.z = v->z;
      if (has_normal_) {
        obj.normal.x = n->x;
        obj.normal.y = n->y;
        obj.normal.z = n->z;
      }
      if (has_texture_) {
        obj.uv.u = uv->x;
        obj.uv.v = uv->y;
      }
      body.push_back(obj);

      min_pos_.x() = std::min(min_pos_.x(), v->x);
      min_pos_.y() = std::min(min_pos_.y(), v->y);
      min_pos_.z() = std::min(min_pos_.z(), v->z);
      max_pos_.x() = std::max(max_pos_.x(), v->x);
      max_pos_.y() = std::max(max_pos_.y(), v->y);
      max_pos_.z() = std::max(max_pos_.z(), v->z);
      
      ++v;
      ++n;
      ++uv;
    }

    // 面情報を生成
    aiFace* f = mesh.mFaces;
    std::vector<Face> face;
    face.reserve(mesh.mNumFaces);

    for (u_int i = 0; i < mesh.mNumFaces; ++i) {
      // 三角ポリゴン以外はエラー
      assert(f->mNumIndices == 3);

      Face obj;
      obj.v1 = f->mIndices[0];
      obj.v2 = f->mIndices[1];
      obj.v3 = f->mIndices[2];
      face.push_back(obj);

      ++f;
    }

    // OpenGLのFrame Buffer Objectを生成して、頂点データを転送する
		glGenBuffers(NUM_VBO, vbo_);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_[ARRAY]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Body) * body.size(), &body[0], GL_STATIC_DRAW);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_[ELEMENT_ARRAY]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Face) * face.size(), &face[0], GL_STATIC_DRAW);

    // バッファの紐づけを解除
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  ~Mesh() {
    DOUT << "~Mesh()" << std::endl;
		glDeleteBuffers(NUM_VBO, vbo_);
  }

  u_int materialIndex() const { return material_index_; }

	GLuint vbo(const Buffer index) const { return vbo_[index]; }
	GLuint points() const { return points_; }
  u_int faces() const { return faces_; }

  const Vec3f& minPos() const { return min_pos_; }
  const Vec3f& maxPos() const { return max_pos_; }
  
};

}
