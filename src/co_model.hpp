
#pragma once

//
// Modelファイル読み込み
// FIXME:テクスチャはpngのみ対応
//

// リンクするライブラリの定義(Windows)
#if defined (_MSC_VER)
#ifdef _DEBUG
#pragma comment (lib, "assimpd.lib")
#else
#pragma comment (lib, "assimp.lib")
#endif
#endif


#include <fstream>
#include <deque>
#include <memory>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "co_mesh.hpp"
#include "co_material.hpp"
#include "co_node.hpp"
#include "co_texMng.hpp"


namespace ngs {

class Model {
  std::deque<std::shared_ptr<Mesh> > meshes_;
  std::deque<Material> materials_;
  Node root_node_;
  std::shared_ptr<TexMng> textures_;

  // 読み込みフラグ
  enum {
    import_flags = aiProcess_JoinIdenticalVertices |
                   aiProcess_Triangulate |
                   aiProcess_FlipUVs |
                   aiProcess_SortByPType |
                   aiProcess_OptimizeMeshes
  };
  
public:
  Model(const std::string& file_name, const std::string& path) :
    textures_(std::make_shared<TexMng>())
  {
    DOUT << "Model()" << std::endl;

    // Open Asset Importerを利用してモデルデータを読み込む
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path + file_name, import_flags);
    if (!scene) {
      DOUT << importer.GetErrorString() << std::endl;
    }
    DOUT << "Mesh:" << scene->mNumMeshes << std::endl;
    DOUT << "Material:" << scene->mNumMaterials << std::endl;

    // メッシュ生成
    for (u_int i = 0; i < scene->mNumMeshes; ++i) {
      const aiMesh& scene_mesh = *(scene->mMeshes[i]);
      // TIPS:コンテナ内に直接Meshを生成する
      meshes_.emplace_back(std::make_shared<Mesh>(scene_mesh));
    }

    // マテリアル生成
    for (u_int i = 0; i < scene->mNumMaterials; ++i) {
      const aiMaterial& scene_material = *(scene->mMaterials[i]);
      materials_.emplace_back(scene_material, *textures_, path);
      // TIPS:コンテナ内に直接Materialを生成する
    }
    
    // 階層構造を生成
    root_node_.setup(scene->mRootNode);
  }

  
  // TIPS:ノード情報を生成しなおす為に自分でコピーする
  Model(const Model& rhs) :
    meshes_(rhs.meshes_),
    materials_(rhs.materials_),
    root_node_(rhs.root_node_),
    textures_(rhs.textures_)
  {
    DOUT << "Model(& model)" << std::endl;
  }

  Model(Model&& rhs) :
    meshes_(std::move(rhs.meshes_)),
    materials_(std::move(rhs.materials_)),
    root_node_(std::move(rhs.root_node_)),
    textures_(std::move(rhs.textures_))
  {
    DOUT << "Model(&& model)" << std::endl;
  }

  ~Model() {
    DOUT << "~Model()" << std::endl;
  }

  
  const std::deque<std::shared_ptr<Mesh> >& mesh() const { return meshes_; }

  const std::deque<Material>& material() const { return materials_; }
  std::deque<Material>& material() { return materials_; }

  const Node& rootNode() const { return root_node_; }
  Node& rootNode() { return root_node_; }

  // モデルに含まれるポリゴン数を返す
  u_int numPolygon() const {
    u_int num = 0;
    for (const auto mesh : meshes_) {
      num += mesh->faces();
    }
    return num;
  }

  // モデルに含まれる全階層数を返す
  u_int numNode() const {
    return root_node_.numNode();
  }


  // マテリアルの反射色を書き換える
  static void materialDiffuseColor(Model& model, const Vec3f& color) {
    std::deque<Material>& materials = model.material();
    for (Material& material : materials) {
      material.diffuse(color);
    }
  }
  
  // マテリアルの発光色を書き換える
  static void materialEmissiveColor(Model& model, const Vec3f& color) {
    std::deque<Material>& materials = model.material();
    for (Material& material : materials) {
      material.emissive(color);
    }
  }
  
  // マテリアルの発光色を書き換える(オリジナルあり)
  static void materialEmissiveColor(Model& model, const std::deque<Material>& original, const Vec3f& color) {
    std::deque<Material>& materials = model.material();
    for (size_t i = 0; i < materials.size(); ++i) {
      materials[i].emissive(original[i].emissive() + color);
    }
  }

  // マテリアルのスペキュラ色を書き換える
  static void materialSpecularColor(Model& model, const Vec3f& color) {
    std::deque<Material>& materials = model.material();
    for (Material& material : materials) {
      material.specular(color);
    }
  }

  // マテリアルのテクスチャを書き換える
  void materialTexture(const std::string& file_name) {
    TexMng::TexPtr texture = textures_->read(file_name);

    for (Material& material : materials_) {
      material.texture(texture);
    }
  }

  // テクスチャ読むだけ
  void readTexture(const std::string& file_name) {
    textures_->read(file_name);
  }
  
};

}
