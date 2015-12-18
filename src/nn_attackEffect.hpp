
#pragma once

//
// 攻撃演出
//

#include "co_miniEasing.hpp"
#include "nn_vbo.hpp"


namespace ngs {

class AttackEffect : public ObjBase {
  Framework& fw_;
  const picojson::value& params_;
  const Camera& camera_;

  std::shared_ptr<EasyShader> shader_;
  
  bool active_;
  bool updated_;
  bool pause_;
  
  float planet_radius_;

  float start_radius_;
  float radius_;
  float width_;
  float div_gap_;
  int div_max_;
  int div_min_;
  
  MiniEasing<float> radius_ease_;
  Ease<GrpCol> color_ease_;
  GrpCol color_;
  
  Quatf rotate_;
  Eigen::Affine3f matrix_;

  int vtx_num_;
  Vbo vtx_vbo_;


public:
  AttackEffect(Framework& fw, const picojson::value& params,
               ShaderHolder& shader_holder, const Camera& camera) :
    fw_(fw),
    params_(params.at("attack_effect")),
    camera_(camera),
    shader_(shader_holder.read(params_.at("shader").get<std::string>())),
    active_(true),
    updated_(false),
    pause_(false),
    width_(params_.at("width").get<double>()),
    div_gap_(params_.at("div_gap").get<double>()),
    div_max_(params_.at("div_max").get<double>()),
    div_min_(params_.at("div_min").get<double>()),
    radius_ease_(miniEasingFromJson<float>(params_.at("radius_ease"))),
    color_ease_(easeFromJson<GrpCol>(params_.at("color_ease")))
  {
    DOUT << "AttackEffect()" << std::endl;
  }

  ~AttackEffect() {
    DOUT << "~AttackEffect()" << std::endl;
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
  // 更新
  void update(const Signal::Params& arguments) {
    if (pause_) return;

    updated_ = true;
    
    if (!radius_ease_.isExec()) {
      active_ = false;
      return;
    }

    float delta_time = boost::any_cast<float>(arguments.at("delta_time"));

    // 半径と色のイージング
    radius_ = start_radius_ + radius_ease_(delta_time);
    color_  = color_ease_(delta_time);

    // 外径と内径
    float radius      = radius_;
    float hole_radius = radius_ - width_;

    // 分割数は半径に比例する
    float circle_len = radius * 2.0f * m_pi;
    int div = minmax(int(circle_len / div_gap_), div_min_, div_max_);
    // DOUT << "Attack div:" << div << std::endl;

    float y      = std::sqrt(planet_radius_ * planet_radius_ - radius * radius) - planet_radius_;
    float hole_y = std::sqrt(planet_radius_ * planet_radius_ - hole_radius * hole_radius) - planet_radius_;

    // 頂点を格納するコンテナを用意。あらかじめ容量を確保しておく
		std::vector<GLfloat> vtx;
    vtx.reserve(div * 3);

		for(int i = 0; i <= div; ++i) {
      // 表面の頂点の並びにするために、角度をマイナス方向に計算
			float r = m_pi * -2.0f * i / div;

			float sin_a = std::sin(r);
			float cos_a = std::cos(r);

			vtx.push_back(radius * sin_a);
      vtx.push_back(y);
			vtx.push_back(radius * cos_a);
			vtx.push_back(hole_radius * sin_a);
      vtx.push_back(hole_y);
			vtx.push_back(hole_radius * cos_a);
		}

    vtx_num_ = static_cast<int>(vtx.size()) / 3;
    
    // VBOに転送
		glBindBuffer(GL_ARRAY_BUFFER, vtx_vbo_.handle());
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vtx.size(), &vtx[0], GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  // 描画
  void draw(const Signal::Params& arguments) {
    if (!updated_) return;

    glDepthMask(GL_FALSE);
    fw_.glState().blend(true);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    pushMatrix();
    multMatrix(matrix_.matrix());

    const EasyShader& shader = *shader_;
    shader();

    const Mat4f& model = getModelMatrix();
    glUniformMatrix4fv(shader.uniform("modelViewMatrix"), 1, GL_FALSE, model.data());

    glUniform4f(shader.uniform("material_diffuse"), color_(0), color_(1), color_(2), color_(3));
    glUniform4f(shader.uniform("material_emissive"), 0.0f, 0.0f, 0.0f, 0.0f);

    glBindBuffer(GL_ARRAY_BUFFER, vtx_vbo_.handle());

    GLint position = shader.attrib("position");
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vtx_num_);

    glDisableVertexAttribArray(position);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    popMatrix();

    fw_.glState().blend(false);
    glDepthMask(GL_TRUE);
  }

  
  // 生成情報を元に初期化
  void setupFromSpawnInfo(const Signal::Params& arguments) {
    planet_radius_ = boost::any_cast<float>(arguments.at("planet_radius")) + 0.1f;
    Vec3f center   = boost::any_cast<Vec3f>(arguments.at("pos"));
    float angle    = boost::any_cast<float>(arguments.at("angle"));
    start_radius_  = planet_radius_ * std::sin(angle);

    rotate_.setFromTwoVectors(Vec3f::UnitY(), center);
    matrix_ =
      rotate_
      * Eigen::Translation<float, 3>(0.0f, planet_radius_, 0.0f);
  }
  
};

}
