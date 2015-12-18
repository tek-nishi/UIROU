
#pragma once

//
// 攻撃範囲表示
//

namespace ngs {

class AttackRange : public ObjBase {
  Framework& fw_;
  const Camera& camera_;

  std::shared_ptr<EasyShader> color_sh_;
  
  bool active_;
  bool updated_;
  bool pause_;
  
  float angle_;
  float radius_;
  float y_pos_;

  float planet_radius_;
  
  float disp_time_;

  Quatf rotate_;
  Eigen::Affine3f matrix_;


public:
  AttackRange(Framework& fw,
              ShaderHolder& shader_holder, const Camera& camera) :
    fw_(fw),
    camera_(camera),
    color_sh_(shader_holder.read("color")),
    active_(true),
    updated_(false),
    pause_(false),
    disp_time_(1.0f)
  {
    DOUT << "AttackRange()" << std::endl;
  }

  ~AttackRange() {
    DOUT << "~AttackRange()" << std::endl;
  }

  
  bool isActive() const {
    return active_;
  }
  
  void message(const int msg, Signal::Params& arguments) {
    if (!active_) return;

    switch (msg) {
    case Msg::UPDATE:
      update(arguments);
      return;

    case Msg::DRAW:
      draw(arguments);
      return;


    case Msg::SET_SPAWN_INFO:
      setupFromSpawnInfo(arguments);
      return;

      
    case Msg::PAUSE_GAME:
      pause_ = true;
      return;

    case Msg::RESUME_GAME:
      pause_ = false;
      return;
      
    case Msg::END_GAME:
      active_ = false;
      return;


    default:
      return;
    }
  }

  
private:
  void update(const Signal::Params& arguments) {
    if (pause_) return;

    updated_ = true;

    float delta_time = boost::any_cast<float>(arguments.at("delta_time"));

    disp_time_ -= delta_time;
    if (disp_time_ <= 0.0f) {
      active_ = false;
      return;
    }
  }

  void draw(const Signal::Params& arguments) {
    // いきなり描画が呼び出された場合には処理しないための措置
    if (!updated_) return;
    
    glDepthMask(GL_FALSE);
    glLineWidth(1.0f);

    pushMatrix();
    multMatrix(matrix_.matrix());

    const EasyShader& shader = *color_sh_;
    shader();
    
    // const Mat4f& projection = getProjectionMatrix();
    // glUniformMatrix4fv(shader.uniform("projectionMatrix"), 1, GL_FALSE, projection.data());

    const Mat4f& model = getModelMatrix();
    glUniformMatrix4fv(shader.uniform("modelViewMatrix"), 1, GL_FALSE, model.data());
    
    glUniform4f(shader.uniform("material_diffuse"), 1.0f, 0.0f, 0.0f, 1.0f);
    glUniform4f(shader.uniform("material_emissive"), 0.0f, 0.0f, 0.0f, 0.0f);

    struct Vtx {
      float x, y, z;
    };
    Vtx vtx[20];
    int vtx_num = ELEMSOF(vtx);
    for (int i = 0; i < vtx_num; ++i) {
      vtx[i].x = std::sin(m_pi * 2 * i / vtx_num) * radius_;
      vtx[i].y = 0.0f;
      vtx[i].z = std::cos(m_pi * 2 * i / vtx_num) * radius_;
    }

    GLint position = shader.attrib("position");
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, sizeof(Vtx), vtx);
    glDrawArrays(GL_LINE_LOOP, 0, vtx_num);
    glDisableVertexAttribArray(position);
    
    popMatrix();
    glDepthMask(GL_TRUE);
  }

  
  // 生成情報を元に初期化
  void setupFromSpawnInfo(const Signal::Params& arguments) {
    planet_radius_ = boost::any_cast<float>(arguments.at("planet_radius"));
    Vec3f center   = boost::any_cast<Vec3f>(arguments.at("pos"));
    angle_         = boost::any_cast<float>(arguments.at("angle"));

    y_pos_  = planet_radius_ * std::cos(angle_);
    radius_ = planet_radius_ * std::sin(angle_);

    rotate_.setFromTwoVectors(Vec3f::UnitY(), center);
    matrix_ =
      rotate_
      * Eigen::Translation<float, 3>(0.0f, y_pos_, 0.0f);
  }
  
};

}
