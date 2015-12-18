
#pragma once

//
// 敵を倒した時に出現するアイテム
//

#include <Eigen/Geometry>
#include "co_easyShader.hpp"
#include "co_modelDraw.hpp"
#include "co_random.hpp"
#include "co_easing.hpp" 
#include "co_miniEasing.hpp"
#include "co_misc.hpp"
#include "nn_modelHolder.hpp"
#include "nn_shaderHolder.hpp"
#include "nn_messages.hpp"
#include "nn_objBase.hpp"


namespace ngs {

class CubeItem : public ObjBase {
  Framework& fw_;
  const picojson::value& params_;
  
  bool active_;
  bool updated_;
  bool pause_;

  u_int hash_;

  // 表示用モデル
  Model model_;
  // オリジナルのマテリアル
  std::deque<Material> materials_;

  // 表示用シェーダー
  std::shared_ptr<EasyShader> shader_;

  float planet_radius_;

  // 大きさ
  float radius_;
  float scale_;

  // 存在時間
  float time_;
  float active_time_;

  // 出現、消滅演出用
  MiniEasing<float> scaling_;

  // 取得時演出用
  MiniEasing<float> y_move_;

  // プレイヤーに引き寄せられる用
  EasingFunc<float> absorb_;
  float absorb_start_;
  float absorb_diff_;
  float absorb_duration_;
  
  bool disappear_;
  
  // くるくる回る演出用
  Vec2f spin_speed_;
  Vec2f spin_angle_;
  
  // 地表からの高さ
  float y_pos_;
  float y_ofs_;
  // 回転
  Quatf rotate_;

  // 位置ベクトル
  Vec3f pos_;
  
  // 表示用行列
  Eigen::Affine3f model_matrix_;

  // アイテム種類、効果時間
  int   type_;
  float effect_time_;
  
  // 色
  Ease<float> color_ease_;
  Vec3f       color_;
  // 効果演出用
  Ease<float> effect_ease_;

  CubeShadow        shadow_;
  Eigen::Affine3f   shadow_matrix_;
  GrpCol            shadow_color_;
  MiniEasing<float> shadow_fade_;
  bool              shadow_disp_;
  
public:
  CubeItem(Framework& fw,
           const picojson::value& params,
           ModelHolder& model_holder, ShaderHolder& shader_holder,
           const CubeShadow& shadow) :
    fw_(fw),
    params_(params.at("cubeItem")),
    active_(true),
    updated_(false),
    pause_(false),
    hash_(createUniqueNumber()),
    model_(model_holder.read(params_.at("model").get<std::string>())),
    materials_(model_.material()),
    shader_(shader_holder.read(params_.at("shader").get<std::string>())),
    radius_(params_.at("radius").get<double>()),
    scale_(radius_ * 2.0f),
    time_(0.0f),
    active_time_(params_.at("exist_time").get<double>()),
    scaling_(miniEasingFromJson<float>(params_.at("entry"))),
    absorb_(params_.at("absorb_type").get<std::string>()),
    absorb_start_(params_.at("absorb_start").get<double>()),
    absorb_diff_(params_.at("absorb_end").get<double>() - absorb_start_),
    absorb_duration_(params_.at("absorb_duration").get<double>() * m_pi),
    disappear_(false),
    spin_speed_(deg2rad(vectFromJson<Vec2f>(params_.at("spin_speed")))),
    spin_angle_(0.0f, 0.0f),
    y_pos_(200.0f),
    y_ofs_(params_.at("y_ofs").get<double>()),
    rotate_(Eigen::AngleAxisf(0.0f, Vec3f::UnitY())),
    model_matrix_(Eigen::Affine3f::Identity()),
    effect_time_(params_.at("effect_time").get<double>()),
    color_ease_(easeFromJson<float>(params_.at("color_disp"))),
    effect_ease_(easeFromJson<float>(params_.at("effect_ease"))),
    shadow_(shadow),
    shadow_matrix_(Eigen::Affine3f::Identity()),
    shadow_color_(vectFromJson<GrpCol>(params_.at("shadow_color"))),
    shadow_fade_(miniEasingFromJson<float>(params_.at("shadow_fade_in"))),
    shadow_disp_(true)
  {
    DOUT << "CubeItem()" << std::endl;

    // タイプと色の設定
    const picojson::array& color(params_.at("color").get<picojson::array>());
    type_ = randomValue(int(color.size()));
    color_ = vectFromJson<Vec3f>(color[type_]);
  }

  ~CubeItem() {
    DOUT << "~CubeItem()" << std::endl;
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

    case Msg::COLLECT_OBJECT_INFO:
      objectInfo(arguments);
      return;

    case Msg::MUTUAL_INTERFERENCE:
      interference(arguments);
      return;

      
    case Msg::DESTROYED_BASE:
      disappear();
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

    // 色アニメーション
    Model::materialEmissiveColor(model_, color_ * color_ease_(delta_time));
    
    y_pos_ = planet_radius_ + y_ofs_;

    float scaling_value = 1.0f;
    if (scaling_.isExec()) {
      // 登場・消滅時の演出
      scaling_value = scaling_(delta_time);
      if (!scaling_.isExec() && disappear_) {
        // 演出が終わったら消滅
        active_ = false;
        return;
      }
    }
    else if (y_move_.isExec()) {
      // アイテムゲット時演出
      y_pos_ += y_move_(delta_time);
      if (!y_move_.isExec() && disappear_) {
        active_ = false;
        return;
      }
    }
    else {
      time_ += delta_time;

      // 有効時間か過ぎたら消滅
      if (time_ >= active_time_) {
        disappear();
      }
    }
    
    spin_angle_ += spin_speed_ * delta_time;
    
    pos_ = rotate_ * Vec3f::UnitY();

    model_matrix_ =
      rotate_
      * Eigen::Translation<float, 3>(0.0f, y_pos_, 0.0f)
      * Eigen::AngleAxisf(spin_angle_.x(), Vec3f::UnitX())
      * Eigen::AngleAxisf(spin_angle_.y(), Vec3f::UnitY())
      * Eigen::Scaling(Vec3f(scale_ * scaling_value, scale_ * scaling_value, scale_ * scaling_value));

    if (shadow_disp_) {
      shadow_matrix_ =
        rotate_
        * Eigen::AngleAxisf(spin_angle_.y(), Vec3f::UnitY())
        * Eigen::Translation<float, 3>(0.0f, planet_radius_, 0.0f);

      // 影の濃さを決める
      float d = (45.0f - minmax(y_pos_ - planet_radius_, 0.0f, 45.0f)) / 45.0f;
      if (shadow_fade_.isExec()) {
        d *= shadow_fade_(delta_time);
        if (!shadow_fade_.isExec() && disappear_) shadow_disp_ = false;
      }
      shadow_.color(shadow_color_ * d);
    }
  }

  // 描画
  void draw(const Signal::Params& arguments) {
    // いきなり描画が呼び出された場合には処理しないための措置
    if (!updated_) return;

    modelDraw(model_, model_matrix_.matrix(), *shader_, true);
    if (shadow_disp_) shadow_.draw(fw_, shadow_matrix_.matrix());
  }


  // 消滅開始
  void disappear() {
    disappear_ = true;

    // 消滅アニメーション開始
    miniEasingFromJson<float>(scaling_, params_.at("disappear"));
    miniEasingFromJson<float>(shadow_fade_, params_.at("shadow_fade_out"));
  }

  
  // 情報を返す
  void objectInfo(Signal::Params& arguments) {  
    if (!updated_ || !canGet()) return;
    
    Msg::ItemInfo info = {
      this, hash_,
      pos_,
      radius_
    };
    arguments.insert(Signal::Params::value_type("item_info", info));
  }

  // 相互干渉
  void interference(const Signal::Params& arguments) {
    if (!updated_) return;

    pickUpItem(arguments);
    absorbToPlayer(arguments);
  }
  
  // プレイヤーに引き寄せられる処理
  void absorbToPlayer(const Signal::Params& arguments) {
    if (!canGet()) return;
    if (!Signal::isParamValue(arguments, "player_info")) return;

    const auto& info = boost::any_cast<const Msg::PlayerInfo&>(arguments.at("player_info"));

    // 距離が近いほど大きく動くのをイージングを使って実現
    float t = minmax(angleFromVecs(pos_, info.pos), 0.0f, absorb_duration_);
    float d = absorb_(t, absorb_start_, absorb_diff_, absorb_duration_);
    
    Quatf r(Quatf::FromTwoVectors(pos_, info.pos));
    Quatf target_rotate = r * rotate_;
    rotate_ = rotate_.slerp(d, target_rotate);
  }

  
  // アイテム取得処理
  void pickUpItem(const Signal::Params& arguments) {
    if (!canGet()) return;
    if (!Signal::isParamValue(arguments, "player_info")) return;

    const auto& info = boost::any_cast<const Msg::PlayerInfo&>(arguments.at("player_info"));
    
    // Vec3f pos = boost::any_cast<Vec3f>(arguments.at("pos"));
    // float radius = boost::any_cast<float>(arguments.at("radius"));

    // 互いが一定以内の距離にあるか判定
    float angle = angleFromVecs(pos_, info.pos);
    float dist = distOnCircle(angle, planet_radius_);

    // 矩形も考慮した距離判定
    if (dist > ((radius_ + info.radius) * 1.4f)) return;

    // ゲット出来たかプレイヤーにお伺いをたてる
    Signal::Params params;
    params.insert(Signal::Params::value_type("item_type", type_));
    params.insert(Signal::Params::value_type("effect_time", effect_time_));
    params.insert(Signal::Params::value_type("effect_ease", effect_ease_));
    params.insert(Signal::Params::value_type("item_color", color_));
    info.obj->message(Msg::PICK_UP_ITEM, params);
    if (Signal::isParamValue(params, "get")) {
      // ゲット
      obtain();
    }
  }

  // アイテムゲット
  void obtain() {
    disappear_ = true;

    // アニメーションきりかえ
    color_ease_ = easeFromJson<float>(params_.at("color_obtain"));
    miniEasingFromJson<float>(y_move_, params_.at("obtain"));
    miniEasingFromJson<float>(shadow_fade_, params_.at("shadow_fade_out"));
  }

  // 設定値を元に初期設定
  void setupFromSpawnInfo(const Signal::Params& arguments) {
    planet_radius_ = boost::any_cast<float>(arguments.at("planet_radius"));

    // 惑星上の座標から行列生成
    Vec3f pos = boost::any_cast<Vec3f>(arguments.at("spawn_pos"));
    rotate_.setFromTwoVectors(Vec3f::UnitY(), pos);

    shadow_.setup(radius_ + params_.at("shadow_radius").get<double>(), planet_radius_, 0.04f);
  }

  bool canGet() const {
    return !scaling_.isExec() && !disappear_;
  }
  
};

}
