
#pragma once

//
// 背景
//

#include <vector>
#include <Eigen/Geometry>
#include "co_easyShader.hpp"
#include "co_modelDraw.hpp"
#include "co_misc.hpp"
#include "nn_modelHolder.hpp"
#include "nn_shaderHolder.hpp"
#include "nn_messages.hpp"
#include "nn_objBase.hpp"


namespace ngs {

class Bg : public ObjBase {
  Framework& fw_;
  const picojson::value& params_;
  Camera& camera_;

  bool active_;
  bool updated_;
  bool pause_;

  std::shared_ptr<EasyShader> shader_;

  struct Vtx {
    float x_, y_;
    
    Vtx(const float x, const float y) :
      x_(x),
      y_(y)
    {}
  };
  std::vector<Vtx> vtx_;

  struct Color {
    float r_, g_, b_, a_;

    Color(const float r, const float g, const float b, const float a) :
      r_(r),
      g_(g),
      b_(b),
      a_(a)
    {}
  };
  std::vector<Color> col_;

  // 白ういろうピンチ演出用
  Vec3f       diffuse_;
  bool        light_effect_;
  Ease<Vec3f> light_effect_ease_;

  
public:
  Bg(Framework& fw, const picojson::value& params,
     Camera& camera, ShaderHolder& shader_holder) :
    fw_(fw),
    params_(params.at("bg")),
    camera_(camera),
    active_(true),
    updated_(false),
    pause_(false),
    shader_(shader_holder.read(params_.at("shader").get<std::string>())),
    diffuse_(0.0f, 0.0f, 0.0f),
    light_effect_(false),
    light_effect_ease_(easeFromJson<Vec3f>(params_.at("light_effect")))
  {
    DOUT << "Bg()" << std::endl;

    // 頂点データ読み込み
    {
      const auto& values = params_.at("vtx").get<picojson::array>();
      for (const auto value : values) {
        Vec2f v = vectFromJson<Vec2f>(value);
        vtx_.emplace_back(v.x(), v.y());
      }
    }

    // 頂点カラー読み込み
    {
      const auto& values = params_.at("color").get<picojson::array>();
      for (const auto value : values) {
        Vec4f v = vectFromJson<Vec4f>(value);
        col_.emplace_back(v(0), v(1), v(2), v(3));
      }
    }
  }

  ~Bg() {
    DOUT << "~Bg()" << std::endl;
  }


  bool isActive() const {
    return active_;
  }
  
  void message(const int msg, Signal::Params& arguments) {
    switch (msg) {
    case Msg::UPDATE:
      update(arguments);
      return;

    case Msg::DRAW:
      draw(arguments);
      return;


    case Msg::DAMAGED_BASE:
      damagedBase(arguments);
      return;

    case Msg::DESTROYED_BASE:
      destroyedBase(arguments);
      return;
      
      
    case Msg::PAUSE_GAME:
      pause_ = true;
      return;

    case Msg::RESUME_GAME:
      pause_ = false;
      return;

    case Msg::ABORT_GAME:
      pause_        = false;
      light_effect_ = false;
      return;
      
    case Msg::END_GAME:
      light_effect_ = false;
      diffuse_      = Vec3f::Zero();
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

    if (light_effect_) {
      diffuse_ = light_effect_ease_(delta_time);

      // ゲーム終了時にループが解除されている
      if (light_effect_ease_.isEnd()) {
        light_effect_ = false;
        diffuse_      = Vec3f::Zero();
      }
    }
  }

  void draw(const Signal::Params& arguments) {
    if (!updated_) return;

    // 現在の透視変換を退避して背景用の透視変換を作成
    setMatrixMode(Matrix::PROJECTION);
    pushMatrix();
    setMatrixMode(Matrix::MODELVIEW);
    pushMatrix();

    camera_.setup();

    const EasyShader& shader = *shader_;
    shader();

    fw_.glState().depthTest(false);
    fw_.glState().cullFace(false);
    
    glUniform4f(shader.uniform("diffuse"), diffuse_(0), diffuse_(1), diffuse_(2), 1.0f);
    
    GLint position = shader.attrib("position");
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, sizeof(Vtx), &vtx_[0]);

    GLint vtx_color = shader.attrib("vtx_color");
    glEnableVertexAttribArray(vtx_color);
    glVertexAttribPointer(vtx_color, 4, GL_FLOAT, GL_FALSE, sizeof(Color), &col_[0]);
    
    // glDrawArrays(GL_TRIANGLE_STRIP, 0, static_cast<GLsizei>(vtx_.size()));
    glDrawArrays(GL_TRIANGLE_FAN, 0, static_cast<GLsizei>(vtx_.size()));

    glDisableVertexAttribArray(position);
    glDisableVertexAttribArray(vtx_color);

    fw_.glState().depthTest(true);
    fw_.glState().cullFace(true);
    
    // 退避してあった透視変換に戻す
		setMatrixMode(Matrix::PROJECTION);
    popMatrix();
    setMatrixMode(Matrix::MODELVIEW);
    popMatrix();
  }

  
  // 白ういろうダメージ
  void damagedBase(const Signal::Params& arguments) {
    float hp_rate = boost::any_cast<float>(arguments.at("base_hp_rate"));

    // ピンチ演出
    if ((hp_rate < 0.5f) && !light_effect_) {
      light_effect_ = true;
      light_effect_ease_.looping(true);
    }
  }
  
  // 白ういろうやられた
  void destroyedBase(const Signal::Params& arguments) {
    // ピンチ演出終了
    light_effect_ease_.looping(false);
  }
  
};

}
