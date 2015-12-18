
#pragma once

//
// 照準
//

#include "co_random.hpp"
#include "co_modelDraw.hpp"


namespace ngs {

class Signt : public ObjBase {
  Framework& fw_;
  const picojson::value& params_;

  bool active_;
  bool updated_;
  bool pause_;

  Model model_;
  std::shared_ptr<EasyShader> shader_;

  // 識別用ハッシュ値
  u_int hash_;

  float planet_radius_;
  
  // 大きさ
  float scale_;

  // 地表からの高さ
  float y_pos_;
  // 回転
  Quatf rotate_;
  // Y軸向き
  float yaw_;

  // 位置ベクトル
  Vec3f pos_;
  // 回転ベクトル
  Vec3f angle_;
  
  // 表示用行列
  Eigen::Affine3f model_matrix_;

  // 登場時の演出
  MiniEasing<float> scale_easing_;
  
  
public:
  Signt(Framework& fw, const picojson::value& params,
        ModelHolder& model_holder, ShaderHolder& shader_holder) :
    fw_(fw),
    params_(params.at("signt")),
    active_(true),
    updated_(false),
    pause_(false),
    model_(model_holder.read(params_.at("model").get<std::string>())),
    shader_(shader_holder.read(params_.at("shader").get<std::string>())),
    scale_(params_.at("scale").get<double>()),
    y_pos_(200.0f),
    rotate_(Eigen::AngleAxisf(randomValue() * m_pi, randomVector<Vec3f>())),
    yaw_(0.0f),
    model_matrix_(Eigen::Affine3f::Identity()),
    scale_easing_(miniEasingFromJson<float>(params_.at("easing")))
  {
    DOUT << "Signt()" << std::endl;
  }

  ~Signt() {
    DOUT << "~Signt()" << std::endl;
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
      // 惑星上の座標から行列生成
      {
        Vec3f pos = boost::any_cast<Vec3f>(arguments.at("target_pos"));
        rotate_.setFromTwoVectors(Vec3f::UnitY(), pos);

        hash_          = boost::any_cast<u_int>(arguments.at("target_hash"));
        planet_radius_ = boost::any_cast<float>(arguments.at("planet_radius"));
      }
      return;

      
    case Msg::TOUCHDOWN_PLANET:
      // プレイヤーが惑星に着地
      if (hash_ == boost::any_cast<u_int>(arguments.at("target_hash"))) {
        active_ = false;
      }
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

    float delta_time = boost::any_cast<float>(arguments.at("delta_time"));

    y_pos_ = planet_radius_;
    yaw_ += deg2rad(float(params_.at("rotate_speed").get<double>())) * delta_time;

    // X-Zのみスケーリング演出
    float scale = scale_;
    if (scale_easing_.isExec()) {
      scale *= scale_easing_(delta_time);
    }
    
    model_matrix_ =
      rotate_
      * Eigen::Translation<float, 3>(0.0f, y_pos_, 0.0f)
      * Eigen::AngleAxisf(yaw_, Vec3f::UnitY())
      * Eigen::Scaling(Vec3f(scale, scale_, scale));
  }

  void draw(const Signal::Params& arguments) {
    if (!updated_) return;

    modelDraw(model_, model_matrix_.matrix(), *shader_, false);
  }


public:
  // 生成位置を計算
  static bool calcSpawnPos(Vec3f& signt_pos, const Vec2f& touch_pos, const View& view, const Camera& camera, const float planet_radius) {
    // タッチ座標(screen)→Window座標
    Vec2f window_pos = view.toWindowPos(touch_pos);

    // 画面最前→最奥へ伸びる線分を計算
    Vec3f pos_near = camera.posToWorld(Vec3f(window_pos.x(), window_pos.y(), 0.0f), Mat4f::Identity());
    Vec3f pos_far  = camera.posToWorld(Vec3f(window_pos.x(), window_pos.y(), 1.0f), Mat4f::Identity());

    // 線分と球の交差した場所→惑星上のタッチ位置
    SphereVolume volume = {
      Vec3f::Zero(),
      planet_radius,
    };
    float res_t;

    if (!testLineSphere(signt_pos, res_t, volume, pos_near, pos_far)) return false;

    return true;
  }
  
};

}
