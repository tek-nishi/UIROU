
#pragma once

//
// 白ういろう
//

#include <Eigen/Geometry>
#include "co_easyShader.hpp"
#include "co_modelDraw.hpp"
#include "co_random.hpp"
#include "co_easing.hpp" 
#include "co_miniEasing.hpp"
#include "co_misc.hpp"
#include "co_miniQuake.hpp"
#include "co_quakeParam.hpp"
#include "nn_modelHolder.hpp"
#include "nn_shaderHolder.hpp"
#include "nn_messages.hpp"
#include "nn_objBase.hpp"
#include "nn_cubeShadow.hpp"
#include "nn_gameSound.hpp"


namespace ngs {

class CubeBase : public ObjBase {
  Framework& fw_;
  const picojson::value& params_;
  
  bool active_;
  bool updated_;
  bool pause_;

#ifdef _DEBUG
  // 不死身
  bool debug_immortal_;
#endif

  u_int hash_;

  // 表示用モデル
  Model model_;
  // オリジナルのマテリアル
  std::deque<Material> materials_;

  // 表示用シェーダー
  std::shared_ptr<EasyShader> shader_;

  // 惑星の大きさ
  float planet_radius_;

  // 大きさ
  float radius_;
  float scale_;

  // 地表からの高さ
  float y_pos_;
  // 回転
  Quatf rotate_;
  // Y軸向き
  float yaw_;

  // 位置ベクトル
  Vec3f pos_;
  // 向きベクトル
  Vec3f angle_;
  
  // 表示用行列
  Eigen::Affine3f model_matrix_;

  // 敵の体当たりに持ちこたえられる回数
  int hp_max_;
  int hp_;

  // ダメージ時の演出
  MiniEasing<Vec3f> hit_effect_;

  // 登場演出
  MiniEasing<float> appear_;

  // 退去
  bool        leave_;
  Ease<float> leave_scale_;

  bool other_destroyed_;
  
  MiniEasing<float> y_move_;

  // アイテム効果
  bool        immortal_;
  float       immortal_time_;
  Ease<Vec3f> immortal_ease_;

  // プレイヤーと接触中
  bool player_contact_;
  
  // 振動
  MiniQuake quake_;
  QuakeParam quake_landing_;
  QuakeParam quake_damage_;
  QuakeParam quake_playerhit_;
  QuakeParam quake_contact_;

  // 表情テクスチャ
  std::string face_normal_;
  std::string face_damage_;
  std::string face_wounded_;

  float face_change_time_;
  
  // 影
  CubeShadow      shadow_;
  Eigen::Affine3f shadow_matrix_;
  GrpCol          shadow_color_;
  
  
public:
  CubeBase(Framework& fw,
           const picojson::value& params,
           ModelHolder& model_holder, ShaderHolder& shader_holder,
           const CubeShadow& shadow) :
    fw_(fw),
    params_(params.at("cubeBase")),
    active_(true),
    updated_(false),
    pause_(false),
#ifdef _DEBUG
    debug_immortal_(false),
#endif
    hash_(createUniqueNumber()),
    model_(model_holder.read(params_.at("model").get<std::string>())),
    // materials_(model_.material()),
    shader_(shader_holder.read(params_.at("shader").get<std::string>())),
    radius_(params_.at("radius").get<double>()),
    scale_(radius_ * 2.0f),
    y_pos_(200.0f),
    rotate_(Eigen::AngleAxisf(0.0f, Vec3f::UnitY())),
    yaw_(0.0f),
    model_matrix_(Eigen::Affine3f::Identity()),
    hp_max_(params_.at("HP").get<double>()),
    hp_(hp_max_),
    hit_effect_(miniEasingFromJson<Vec3f>(params_.at("hit_effect"))),
    appear_(miniEasingFromJson<float>(params_.at("appear"))),
    leave_(false),
    leave_scale_(easeFromJson<float>(params_.at("leave_scale"))),
    other_destroyed_(false),
    y_move_(miniEasingFromJson<float>(params_.at("entry"))),
    immortal_(false),
    immortal_ease_(easeFromJson<Vec3f>(params_.at("immortal_ease"))),
    player_contact_(false),
    quake_landing_(params_.at("quake_landing")),
    quake_damage_(params_.at("quake_damage")),
    quake_playerhit_(params_.at("quake_playerhit")),
    quake_contact_(params_.at("quake_contact")),
    face_damage_(params_.at("face_damage").get<std::string>()),
    face_wounded_(params_.at("face_wounded").get<std::string>()),
    face_change_time_(0.0f),
    shadow_(shadow),
    shadow_matrix_(Eigen::Affine3f::Identity()),
    shadow_color_(vectFromJson<GrpCol>(params_.at("shadow_color")))
  {
    DOUT << "CubeBase()" << std::endl;

    // 表情テクスチャ(ランダムで決める)
    const auto& face_textures = params_.at("face_normal").get<picojson::array>();
    face_normal_ = face_textures.at(randomValue(static_cast<int>(face_textures.size()))).get<std::string>();
    model_.materialTexture(fw_.loadPath() + face_normal_);

    model_.readTexture(fw_.loadPath() + face_damage_);
    model_.readTexture(fw_.loadPath() + face_wounded_);
    
    // FIXME:マテリアルのdeffuseだけ書き換える(colladaがテクスチャ付きのに対応していない)
    Model::materialDiffuseColor(model_, vectFromJson<Vec3f>(params_.at("material_deffuse")));
    materials_ = model_.material();

    // ダメージ効果演出は止めておく
    hit_effect_.stop();
  }

  ~CubeBase() {
    DOUT << "~CubeBase()" << std::endl;
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

    case Msg::TOUCHDOWN_PLANET:
      hitCheckWithPlayer(arguments);
      return;

    case Msg::ITEM_BASE_IMMORTAL:
      itemBaseImmortal(arguments);
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

    y_pos_ = planet_radius_ - 1.0f;

    if (y_move_.isExec()) {
      // 登場時と去る時の演出
      y_pos_ += y_move_(delta_time);

      if (!y_move_.isExec()) {
        if (leave_) {
          // 消滅
          active_ = false;
          return;
        }
        else {
#if 0
          // 惑星に着陸したら、プレイヤーを生成
          Signal::Params params;
          params.insert(Signal::Params::value_type("spawn_rotate", rotate_));
          fw_.signal().sendMessage(Msg::SPAWN_PLAYER, params);
#endif

          // 着地の振動
          quake_landing_.start(quake_);
          // SE
          gamesound::play(fw_, "base_landing");
        }
      }
    }
    
    pos_   = rotate_ * Vec3f::UnitY();
    angle_ = rotate_ * Vec3f::UnitX();

    // yaw_ += m_pi * 0.1f * delta_time;

    // アイテム効果
    if (immortal_) {
      immortal_time_ -= delta_time;
      immortal_ = (immortal_time_ > 0.0f);

      Model::materialEmissiveColor(model_, materials_, immortal_ ? immortal_ease_(delta_time) : Vec3f::Zero());
    }
    else if (hit_effect_.isExec()) {
      Model::materialEmissiveColor(model_, materials_, hit_effect_(delta_time));
    }

    // 表情変化
    if (face_change_time_ > 0.0f) {
      face_change_time_ -= delta_time;
      if (face_change_time_ <= 0.0f) {
        model_.materialTexture(face_normal_);
      }
    }
    
    // 振動効果
    float quake_scale = quake_(delta_time) * scale_;

    float scale = scale_;
    if (appear_.isExec()) {
      // 登場時
      scale *= appear_(delta_time);
    }
    else if (leave_) {
      // 退去
      scale *= leave_scale_(delta_time);
    }
    
    model_matrix_ =
      rotate_
      * Eigen::Translation<float, 3>(0.0f, y_pos_, 0.0f)
      // * Eigen::AngleAxisf(yaw_, Vec3f::UnitY())
      * Eigen::Scaling(Vec3f(scale + quake_scale, scale - quake_scale, scale + quake_scale));

    shadow_matrix_ =
      rotate_
      * Eigen::Translation<float, 3>(0.0f, planet_radius_, 0.0f);

    // 影の濃さを決める
    float d = (45.0f - minmax(y_pos_ - planet_radius_, 0.0f, 45.0f)) / 45.0f;
    shadow_.color(shadow_color_ * d);

#ifdef _DEBUG
    char key = fw_.keyboard().getPushed();
    if (key == 'D') {
      // 強制GameOver
      hp_ = 0;

      if (!other_destroyed_) {
        Signal::Params param;
        param.insert(Signal::Params::value_type("base_hp", hp_));
        param.insert(Signal::Params::value_type("base_hash", hash_));
        fw_.signal().sendMessage(Msg::DESTROYED_BASE, param);
      }
    }
    else if (key == 'I') {
      // 不死身 ON
      debug_immortal_ = true;
      DOUT << "Base immortal: ON" << std::endl;
    }
    else if (key == 'i') {
      // 不死身 OFF
      debug_immortal_ = false;
      DOUT << "Base immortal: OFF" << std::endl;
    }
#endif
  }

  // 描画
  void draw(const Signal::Params& arguments) {
    // いきなり描画が呼び出された場合には処理しないための措置
    if (!updated_) return;

    modelDraw(model_, model_matrix_.matrix(), *shader_, true);
    shadow_.draw(fw_, shadow_matrix_.matrix());
  }

  
  // 生成情報を元に初期化
  void setupFromSpawnInfo(const Signal::Params& arguments) {
    planet_radius_ = boost::any_cast<float>(arguments.at("planet_radius"));

    if (Signal::isParamValue(arguments, "spawn_pos")) {
      Vec3f pos = boost::any_cast<Vec3f>(arguments.at("spawn_pos"));
      rotate_.setFromTwoVectors(Vec3f::UnitY(), pos);
    }

    shadow_.setup(radius_ + params_.at("shadow_radius").get<double>(), planet_radius_, 0.2f);
  }
  
  // 情報を返す
  void objectInfo(Signal::Params& arguments) {
    if (!updated_ || (hp_ == 0)) return;

    Msg::BaseInfo info = {
      this,
      hash_,
      pos_, angle_, rotate_,
      radius_, scale_,
      hp_, hp_max_
    };
    Signal::pushbackParamValue(arguments, "base_info", info);
  }

  // 相互干渉
  void interference(const Signal::Params& arguments) {
    if (!updated_) return;

    Signal::Params params;

    // 相手のボリュームをスケーリングも含めた行列で変換するので、
    // Volumeには、スケーリングする前の値を指示
    AABBVolume volume = {
      Vec3f(0.0f, 0.5f, 0.0f),
      Vec3f(0.5f, 0.5f, 0.5f)
    };
    params.insert(Signal::Params::value_type("volume", volume));
    params.insert(Signal::Params::value_type("rev_matrix", model_matrix_.inverse()));
    params.insert(Signal::Params::value_type("scale", scale_));
    params.insert(Signal::Params::value_type("pos", pos_));
    params.insert(Signal::Params::value_type("radius", radius_));

    // プレイヤーとの接触
    if (Signal::isParamValue(arguments, "player_info")) {
      const auto& info = boost::any_cast<const Msg::PlayerInfo&>(arguments.at("player_info"));
      info.obj->message(Msg::CHECK_HIT_BASE, params);

      bool contact = boost::any_cast<bool>(params.at("contact"));
      if ((contact != player_contact_) && !quake_.isExec()) {
        quake_contact_.start(quake_);
        
        // SE
        gamesound::play(fw_, "player_hit");
      }
      player_contact_ = contact;
    }
    
    // 敵が存在しなければここで終了
    if (!Signal::isParamValue(arguments, "enemy_info")) return;
    
    // 全ての敵に対して接触判定
    // FIXME:このループを無くしたい
    const auto& infos = boost::any_cast<const std::deque<Msg::EnemyInfo>&>(arguments.at("enemy_info"));
    for (const auto& info : infos) {
      info.obj->message(Msg::CHECK_HIT_BASE, params);
    }
    
    const auto it = params.find("hit_num");
    if (it != params.cend()) {
      // 敵が接触した
      int hit_num = boost::any_cast<int>(it->second);
      DOUT << "Enemy hit:" << hit_num << std::endl;

      // ダメージ演出
      quake_damage_.start(quake_);
      if (!immortal_) {
        hit_effect_.restart();
        model_.materialTexture(face_damage_);
        face_change_time_ = static_cast<float>(params_.at("face_damage_time").get<double>());
      }

      if (hp_ > 0) {
#ifdef _DEBUG
        // 不死身モード
        if (debug_immortal_) hp_ += hit_num;
#endif
        // アイテム効果
        if (immortal_) hp_ += hit_num;

        hp_ -= hit_num;
        if (hp_ <= 0) {
          // 基地が破壊された
          hp_ = 0;

          // 去る演出開始
          leave_ = true;
          miniEasingFromJson<float>(y_move_, params_.at("leave"));

          // 最初にやられたのだけメッセージを投げる
          if (!other_destroyed_) {
            Signal::Params param;
            param.insert(Signal::Params::value_type("base_hp", hp_));
            param.insert(Signal::Params::value_type("base_hash", hash_));
            fw_.signal().sendMessage(Msg::DESTROYED_BASE, param);

            // SE
            gamesound::play(fw_, "base_destroyed");
          }
        }
        else {
          // ダメージを受けた
          float hp_rate = float(hp_) / hp_max_;
          if (hp_rate < 0.5f) face_normal_ = face_wounded_;
          
          Signal::Params param;
          param.insert(Signal::Params::value_type("base_hp", hp_));
          param.insert(Signal::Params::value_type("base_hash", hash_));
          param.insert(Signal::Params::value_type("base_hp_rate", hp_rate));
          fw_.signal().sendMessage(Msg::DAMAGED_BASE, param);

          // SE
          gamesound::play(fw_, "enemy_hit");
        }
        hpToColor();
      }
    }
  }

  // Playerの攻撃との接触判定
  void hitCheckWithPlayer(const Signal::Params& arguments) {
    bool collision = abs(y_pos_ - planet_radius_) < 1.5f;
    if (!collision || leave_) return;

    // TIPS:Cubeの大きさを考慮した角度差
    const Vec3f& pos = boost::any_cast<Vec3f>(arguments.at("pos"));
    float angle = angleFromVecs(pos, pos_);
    angle -= angleOnCircle(1.4f * radius_, planet_radius_);

    if (angle < boost::any_cast<float>(arguments.at("angle"))) {
      // 攻撃HIT
      quake_playerhit_.start(quake_);

      // SE
      gamesound::play(fw_, "player_hit");
    }
  }

  // アイテム効果: 白ういろう無敵
  void itemBaseImmortal(const Signal::Params& arguments) {
    immortal_      = true;
    immortal_time_ = boost::any_cast<float>(arguments.at("effect_time"));
    immortal_ease_.toStart();
    
    hit_effect_.stop();
  }

  // 白ういろうがやられた
  void destroyedBase(const Signal::Params& arguments) {
    other_destroyed_ = true;
  }

  
  // HPが減るにつれて色が黒くなる効果
  void hpToColor() {
    float rate = float(hp_) / hp_max_;
    std::deque<Material>& model_materials = model_.material();
    for (u_int i = 0; i < materials_.size(); ++i) {
      Vec3f col(materials_[i].diffuse() * rate);
      model_materials[i].diffuse(col);
    }
  }
  
};

}
