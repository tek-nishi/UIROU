
#pragma once

//
// Model表示
//

#include "co_model.hpp"
#include "co_easyShader.hpp"


namespace ngs {

// メッシュの描画
void meshDraw(const Mesh& mesh, const EasyShader& shader,
              const bool use_texture, const bool lighting) {
  glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo(Mesh::ARRAY));

  // 頂点
  GLint position_hdl = shader.attrib("position");
  glEnableVertexAttribArray(position_hdl);
  glVertexAttribPointer(position_hdl, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Body), 0);

  // 法線
  GLint normal_hdl = 0;
  if (lighting) {
    normal_hdl = shader.attrib("normal");
    glEnableVertexAttribArray(normal_hdl);
    glVertexAttribPointer(normal_hdl, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Body), (GLvoid *)offsetof(Mesh::Body, normal));
  }

  // UV(テクスチャがあれば)
  GLint uv_hdl = 0;
  if (use_texture) {
    uv_hdl = shader.attrib("uv");
    glEnableVertexAttribArray(uv_hdl);
    glVertexAttribPointer(uv_hdl, 2, GL_FLOAT, GL_FALSE, sizeof(Mesh::Body), (GLvoid *)offsetof(Mesh::Body, uv));
  }

  // 面ごとの頂点インデックス配列を使って描画
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbo(Mesh::ELEMENT_ARRAY));
  glDrawElements(GL_TRIANGLES, mesh.points(), GL_UNSIGNED_SHORT, 0);

  // 割り当てを解除しておく
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // 頂点バッファへの関連付けも解除
  glDisableVertexAttribArray(position_hdl);
  if (lighting) {
    glDisableVertexAttribArray(normal_hdl);
  }
  if (use_texture) {
    glDisableVertexAttribArray(uv_hdl);
  }
}


// ノードごとのシェーダーの行列のセットアップ
void setupMatrix(const EasyShader& shader,
                 const Mat4f& model_projection, const Mat3f& normal,
                 const bool lighting) {
  shader();
  
  // glUniformMatrix4fv(shader.uniform("modelViewProjectionMatrix"), 1, GL_FALSE, model_projection.data());
  if (lighting) {
    glUniformMatrix3fv(shader.uniform("normalMatrix"), 1, GL_FALSE, normal.data());
  }
}


void setupModelingMatrix(const EasyShader& shader, const Mat4f& matrix, const Mat3f& normal, const bool lighting) {
  glUniformMatrix4fv(shader.uniform("modelViewMatrix"), 1, GL_FALSE, matrix.data());
  if (lighting) {
    glUniformMatrix3fv(shader.uniform("normalMatrix"), 1, GL_FALSE, normal.data());
  }
}


void setupProjectionMatrix(const EasyShader& shader, const Mat4f& matrix) {
  shader();
  glUniformMatrix4fv(shader.uniform("projectionMatrix"), 1, GL_FALSE, matrix.data());
}


void setupShadowMatrix(const EasyShader& shader, const Mat4f& matrix) {
  Mat4f bias_mtx;
  bias_mtx << 
    0.5, 0.0, 0.0, 0.5,
    0.0, 0.5, 0.0, 0.5,
    0.0, 0.0, 0.5, 0.5,
    0.0, 0.0, 0.0, 1.0;
    
  Mat4f depth_mtx;
  depth_mtx = bias_mtx * matrix;
  shader();
  glUniformMatrix4fv(shader.uniform("shadowMatrix"), 1, GL_FALSE, depth_mtx.data());
}


// マテリアルごとのシェーダーのセットアップ
void setupShader(const EasyShader& shader,
                 const Material& material,
                 const bool use_texture, const bool lighting) {
  if (use_texture) {
    glUniform1i(shader.uniform("sampler"), 0);
    material.bindTexture();
  }

  {
    const Vec3f& diffuse = material.diffuse();
    glUniform4f(shader.uniform("material_diffuse"), diffuse.x(), diffuse.y(), diffuse.z(), 1.0f);
  }

  {
    const Vec3f& emissive = material.emissive();
    glUniform4f(shader.uniform("material_emissive"), emissive.x(), emissive.y(), emissive.z(), 0.0f);
  }
  
  if (lighting) {
    const Vec3f& specular = material.specular();
    glUniform4f(shader.uniform("material_specular"), specular.x(), specular.y(), specular.z(), 0.0f);
    glUniform1f(shader.uniform("material_shininess"), material.shininess());
  }
}

// ノードに含まれるメッシュを描画
// ノードの親子関係から再帰的に呼ばれる
void nodeDraw(Node& node,
              const Mat4f& matrix,
              const EasyShader& shader_color, const EasyShader& shader_texture,
              const bool lighting,
              const std::deque<std::shared_ptr<Mesh> >& mesh,
              const std::deque<Material>& material) {
  // ノードが非表示な場合は子ノードもまとめて表示しない
  if (!node.display()) return;
  
  pushMatrix();
  multMatrix(node.matrix());
  Mat4f model_local = matrix * node.matrix();

  {
    Mat4f& model = getModelMatrix();

    // 光源はモデル座標系で行う
    Mat3f normal = model_local.block(0, 0, 3, 3);

    shader_color();
    setupModelingMatrix(shader_color, model, normal, lighting);

    shader_texture();
    setupModelingMatrix(shader_texture, model, normal, lighting);
  }

  // ノードに含まれるメッシュを順番に描画
  for (const u_int mesh_index : node.meshIndexes()) {
    const Mesh&     l_mesh     = *mesh[mesh_index];
    const Material& l_material = material[l_mesh.materialIndex()];

    bool use_texture = l_material.texture();
    const EasyShader& shader = use_texture ? shader_texture : shader_color;

    shader();
    setupShader(shader, l_material, use_texture, lighting);

    meshDraw(l_mesh, shader, use_texture, lighting);
      
    if (use_texture) {
      glBindTexture(GL_TEXTURE_2D, 0);
    }
  }
    
  // 子ノードを再帰的に描画
  for (auto& child : node.childs()) {
    nodeDraw(child,
             model_local,
             shader_color, shader_texture,
             lighting,
             mesh, material);
  }
  
  popMatrix();
}


// モデルの描画
void modelDraw(Model& model,
               const Mat4f& matrix,
               const EasyShader& shader_color, const EasyShader& shader_texture,
               const bool lighting = false) {
  pushMatrix();
  multMatrix(matrix);
  
  nodeDraw(model.rootNode(),
           matrix,
           shader_color, shader_texture,
           lighting,
           model.mesh(), model.material());

  popMatrix();
}


// ノードに含まれるメッシュを描画
// ノードの親子関係から再帰的に呼ばれる
// シェーダー１つVer.
void nodeDraw(Node& node,
              const Mat4f& matrix,
              const EasyShader& shader,
              const bool lighting,
              const std::deque<std::shared_ptr<Mesh> >& mesh,
              const std::deque<Material>& material) {
  // ノードが非表示な場合は子ノードもまとめて表示しない
  if (!node.display()) return;
  
  pushMatrix();
  multMatrix(node.matrix());
  Mat4f model_local = matrix * node.matrix();

  {
    Mat4f& model = getModelMatrix();

    // 光源はモデル座標系で行う
    Mat3f normal = model_local.block(0, 0, 3, 3);

    shader();
    setupModelingMatrix(shader, model, normal, lighting);
  }

  // ノードに含まれるメッシュを順番に描画
  for (const u_int mesh_index : node.meshIndexes()) {
    const Mesh&     l_mesh     = *mesh[mesh_index];
    const Material& l_material = material[l_mesh.materialIndex()];

    bool use_texture = l_material.texture();
    setupShader(shader, l_material, use_texture, lighting);
    meshDraw(l_mesh, shader, use_texture, lighting);

#if 0
    if (use_texture) {
      glBindTexture(GL_TEXTURE_2D, 0);
    }
#endif
  }
    
  // 子ノードを再帰的に描画
  for (auto& child : node.childs()) {
    nodeDraw(child,
             model_local,
             shader,
             lighting,
             mesh, material);
  }
  
  popMatrix();
}



// モデルの描画
// シェーダー１つVer.
void modelDraw(Model& model,
               const Mat4f& matrix,
               const EasyShader& shader,
               const bool lighting = false) {
  pushMatrix();
  multMatrix(matrix);

  shader();
  
  nodeDraw(model.rootNode(),
           matrix,
           shader,
           lighting,
           model.mesh(), model.material());

  popMatrix();
}

}
