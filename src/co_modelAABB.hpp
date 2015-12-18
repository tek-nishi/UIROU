
#pragma once

//
// Model AABB class
//

#include "co_defines.hpp"
#include <deque>
#include <unordered_map>
#include <string>
#include "co_model.hpp"
#include "co_collision.hpp"


namespace ngs {

class ModelAABB {
public:
  struct Body {
    Mat4f matrix;
    AABBVolume volume;
  };

  
private:
  std::unordered_map<std::string, Body> volume_;

  
public:
  explicit ModelAABB(const Model& model) {
    checkMeshVolume(model.rootNode(), Mat4f::Identity(), model.mesh());
  }

  const Body& volume(const std::string& name) const {
    const auto it = volume_.find(name);
    assert(it != volume_.cend());
    return it->second;
  }
  
  Body& volume(const std::string& name) {
    const auto it = volume_.find(name);
    assert(it != volume_.cend());
    return it->second;
  }

  bool isVolume(const std::string& name) const {
    const auto it = volume_.find(name);
    return it != volume_.cend();
  }

  const std::unordered_map<std::string, Body>& allVolumes() const {
    return volume_;
  }

  void updateMatrix(const Model& model) {
    updateNodeMatrix(model.rootNode(), Mat4f::Identity());
  }

  
private:
  void checkMeshVolume(const Node& node, const Mat4f parent_matrix, const std::deque<std::shared_ptr<Mesh> >& meshes) {
    Mat4f current_matrix = parent_matrix * node.matrix();

    const std::deque<u_int>& mesh_index = node.meshIndexes();
    if (!mesh_index.empty()) {
      // FIXME:最小値、最大値はC++的な定義がある
      Vec3f min_pos(FLT_MAX, FLT_MAX, FLT_MAX);
      Vec3f max_pos(-FLT_MAX, -FLT_MAX, -FLT_MAX);

      // Nodeに含まれる全てのMeshからAABBを決定
      for (const u_int index : mesh_index) {
        std::shared_ptr<Mesh> mesh = meshes[index];
        const Vec3f& mesh_min_pos = mesh->minPos();
        const Vec3f& mesh_max_pos = mesh->maxPos();

        min_pos.x() = std::min(min_pos.x(), mesh_min_pos.x());
        min_pos.y() = std::min(min_pos.y(), mesh_min_pos.y());
        min_pos.z() = std::min(min_pos.z(), mesh_min_pos.z());
        max_pos.x() = std::max(max_pos.x(), mesh_max_pos.x());
        max_pos.y() = std::max(max_pos.y(), mesh_max_pos.y());
        max_pos.z() = std::max(max_pos.z(), mesh_max_pos.z());
      }

      Body body = {
        current_matrix,
        {
          (min_pos + max_pos) / 2.0f,
          (max_pos - min_pos) / 2.0f
        }
      };
      volume_.insert(std::unordered_map<std::string, Body>::value_type(node.name(), body));
    }

    // 再帰的に子ノードのAABBも作成
    const std::vector<Node>& child = node.childs();
    for (const Node& child_node : child) {
      checkMeshVolume(child_node, current_matrix, meshes);
    }
  }

  void updateNodeMatrix(const Node& node, Mat4f parent_matrix) {
    Mat4f current_matrix = parent_matrix * node.matrix();
    auto it = volume_.find(node.name());
    // 全てのNodeがAABBを持っているとは限らない
    if (it != volume_.end()) {
      it->second.matrix = current_matrix;
    }
    
    // 再帰的に子ノードの行列も更新
    const std::vector<Node>& child = node.childs();
    for (const Node& child_node : child) {
      updateNodeMatrix(child_node, current_matrix);
    }
  }
  
  
};

}
