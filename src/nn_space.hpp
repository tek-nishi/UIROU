
#pragma once

//
// 宇宙空間的な遠景
//

#include "co_random.hpp"


namespace ngs {

class Space : public ObjBase {
  Framework& fw_;
  const picojson::value& params_;
  const Camera& camera_;
  
  bool active_;
  bool updated_;
  bool pause_;

  Model model_;
  std::shared_ptr<EasyShader> shader_;

  Vec3f rotate_vec_;
  float rotate_speed_;
  
  Vec3f pos_;
  Quatf rotate_;
  float scale_;

  
public:
  Space(Framework& fw, const picojson::value& params,
        const Camera& camera,
        ModelHolder& model_holder, ShaderHolder& shader_holder) :
    fw_(fw),
    params_(params.at("space")),
    camera_(camera),
    active_(true),
    updated_(false),
    pause_(false),
    model_(model_holder.read(params_.at("model").get<std::string>())),
    shader_(shader_holder.read(params_.at("shader").get<std::string>())),
    rotate_vec_(randomVector<Vec3f>()),
    rotate_speed_(deg2rad(randomValue(vectFromJson<Vec2f>(params_.at("rotate_speed"))))),
    pos_(vectFromJson<Vec3f>(params_.at("pos"))),
    rotate_(Quatf::Identity()),
    scale_(params_.at("scale").get<double>())
  {
    DOUT << "Space()" << std::endl;

    // FIXME:マテリアルのdeffuseだけ書き換える(colladaがテクスチャ付きのに対応していない)
    Model::materialDiffuseColor(model_, vectFromJson<Vec3f>(params_.at("material_deffuse")));
  }

  ~Space() {
    DOUT << "~Space()" << std::endl;
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

      
    case Msg::PAUSE_GAME:
      pause_ = true;
      return;

    case Msg::RESUME_GAME:
      pause_ = false;
      return;

    case Msg::END_GAME:
      pause_ = false;
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

    Quatf q(Eigen::AngleAxisf(rotate_speed_ * delta_time, rotate_vec_));
    rotate_ = q * rotate_;
  }

  void draw(const Signal::Params& arguments) {
    if (!updated_) return;

    fw_.glState().depthTest(false);
    fw_.glState().blend(true);

    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    pushMatrix();
    loadIdentity();

    // カメラの向きだけを使うので、表示行列はupdateではなく、ここで計算する
    Eigen::Affine3f m;
    m =
      Eigen::Translation<float, 3>(pos_)
      * camera_.rotate()
      * rotate_
      * Eigen::Scaling(Vec3f(scale_, scale_, scale_));

    modelDraw(model_, m.matrix(), *shader_, true);

    popMatrix();

    fw_.glState().depthTest(true);
    fw_.glState().blend(false);
  }
  
};

}
