
#pragma once

//
// プレイヤーが操作する立方体
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
#include "gamecenter.h"


namespace ngs {

class CubePlayer : public ObjBase {
  Framework& fw_;
  const picojson::value& params_;
  const Camera& camera_;
#ifdef _DEBUG
  // おおもとのパラメーター
  const picojson::value& json_;
#endif

  bool active_;
  bool updated_;
  bool pause_;
  bool demo_mode_;

  u_int hash_;

  Model model_;
  // オリジナルのマテリアル
  std::deque<Material> materials_;

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

  // 進行速度
  float speed_;

  // 位置ベクトル
  Vec3f pos_;
  // 回転ベクトル
  Vec3f angle_;
  
  // 表示用行列
  Eigen::Affine3f model_matrix_;

  // 照準数
  int signt_ready_num_;
  int signt_ready_max_;
  
  // 照準回復時間
  float signt_ready_time_;
  std::deque<float> signt_ready_queue_;
  
  // 目標
  bool       to_target_;
  u_int      target_hash_;
  Vec3f      target_pos_;
  QuatEasing target_rotate_;
  float      target_yaw_;

  bool landing_;

  // 登場演出
  MiniEasing<float> appear_;
  MiniEasing<float> entry_;
  
  float attack_speed_;
  float attack_height_;
  float attack_distance_;
  Vec2f attack_range_;
  
  MiniEasing<float> target_;
  float target_height_;
  float target_power_;

  bool  stiff_;
  float stiff_time_;

  int signt_hash_;
  struct SigntInfo {
    u_int hash;
    Vec3f pos;
  };
  std::deque<SigntInfo> signt_on_planet_;

  // プレイ情報
  struct Info {
    int attack_num;
    int attack_hit_num;
    int attack_combo_num;
    int attack_combo_max;

    Info() :
      attack_num(0),
      attack_hit_num(0),
      attack_combo_num(0),
      attack_combo_max(0)
    {}
  };
  Info in_game_;
  Info results_;
  
  // アイテム効果
  bool  attack_max_power_;
  float attack_max_power_time_;
  bool  attack_max_range_;
  float attack_max_range_time_;

  bool        item_effect_;
  Vec3f       item_effect_color_;
  float       item_effect_time_;
  Ease<float> item_effect_ease_;
  
  // 振動
  MiniQuake  quake_;
  QuakeParam quake_landing_;
  QuakeParam quake_attack_;
  QuakeParam quake_move_;
  float      quake_move_rate_;

  // 表情
  std::string face_normal_;
  std::string face_attack_;

  float face_change_time_;
  
  // 影
  CubeShadow      shadow_;
  Eigen::Affine3f shadow_matrix_;
  GrpCol          shadow_color_;

  
public:
  CubePlayer(Framework& fw,
             const picojson::value& params,
             const Camera& camera,
             ModelHolder& model_holder, ShaderHolder& shader_holder,
             const CubeShadow& shadow) :
    fw_(fw),
    params_(params.at("cubePlayer")),
    camera_(camera),
#ifdef _DEBUG
    json_(params),
#endif
    active_(true),
    updated_(false),
    pause_(false),
    demo_mode_(false),
    hash_(createUniqueNumber()),
    model_(model_holder.read(params_.at("model").get<std::string>())),
    // materials_(model_.material()),
    shader_(shader_holder.read(params_.at("shader").get<std::string>())),
    radius_(params_.at("radius").get<double>()),
    scale_(radius_ * 2.0f),
    y_pos_(200.0f),
    rotate_(Quatf::Identity()),
    yaw_(0.0f),
    speed_(params_.at("speed").get<double>()),
    model_matrix_(Eigen::Affine3f::Identity()),
    signt_ready_num_(params_.at("signt_ready_num").get<double>()),
    signt_ready_max_(signt_ready_num_),
    signt_ready_time_(params_.at("signt_ready_time").get<double>()),
    to_target_(false),
    landing_(false),
    appear_(miniEasingFromJson<float>(params_.at("appear"))),
    entry_(miniEasingFromJson<float>(params_.at("entry"))),
    attack_speed_(params_.at("attack_speed").get<double>()),
    attack_height_(params_.at("attack_height").get<double>()),
    attack_distance_(params_.at("attack_distance").get<double>()),
    attack_range_(vectFromJson<Vec2f>(params_.at("attack_range"))),
    item_effect_(false),
    stiff_(false),
    signt_hash_(0),
    attack_max_power_(false),
    attack_max_range_(false),
    quake_landing_(params_.at("quake_landing")),
    quake_attack_(params_.at("quake_attack")),
    quake_move_(params_.at("quake_move")),
    quake_move_rate_(params_.at("quake_move_rate").get<double>()),
    face_normal_(params_.at("face_normal").get<std::string>()),
    face_attack_(params_.at("face_attack").get<std::string>()),
    face_change_time_(0.0f),
    shadow_(shadow),
    shadow_matrix_(Eigen::Affine3f::Identity()),
    shadow_color_(vectFromJson<GrpCol>(params_.at("shadow_color")))
  {
    DOUT << "CubePlayer()" << std::endl;

    // 表情読み込み
    model_.readTexture(fw_.loadPath() + face_attack_);
    
    // FIXME:マテリアルのdeffuseだけ書き換える(colladaがテクスチャ付きのに対応していない)
    Model::materialDiffuseColor(model_, vectFromJson<Vec3f>(params_.at("material_deffuse")));
    materials_ = model_.material();
  }

  ~CubePlayer() {
    DOUT << "~CubePlayer()" << std::endl;
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

      
    case Msg::START_JUMPATTACK:
      targetOnPlanet(arguments);
      return;

      
    case Msg::COLLECT_OBJECT_INFO:
      objectInfo(arguments);
      return;

    case Msg::MUTUAL_INTERFERENCE:
      interference(arguments);
      return;

      
    case Msg::CHECK_HIT_BASE:
      hitCheckWithBase(arguments);
      return;
      
    case Msg::PICK_UP_ITEM:
      pickUpItem(arguments);
      return;

    case Msg::ITEM_MAX_POWER:
      itemMaxPower(arguments);
      return;

    case Msg::ITEM_MAX_RANGE:
      itemMaxRange(arguments);
      return;

      
    case Msg::START_GAMEMAIN:
      startGameMain();
      return;

#if 0
    case Msg::DAMAGED_BASE:
      hpToColor(boost::any_cast<float>(arguments.at("base_hp_rate")));
      return;
#endif

    case Msg::DESTROYED_BASE:
      // 各種データをコピー(ゲーム終了後に加算されるのを防ぐ)
      results_ = in_game_;
      return;

    case Msg::GATHER_GAME_RESULT:
      gatherGameResult(arguments);
      return;

      
    case Msg::PLAYER_RECORD_INFO:
      recordInfo(arguments);
      return;

    case Msg::PLAYER_PLAYBACK_INFO:
      playbackInfo(arguments);
      return;

      
    case Msg::EXEC_DEMO_MODE:
      execDemoMode(arguments);
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

    y_pos_ = planet_radius_ - 0.5f;

    // 照準の残りを増やす
    if (!signt_ready_queue_.empty()) {
      // キューに積まれた回復時間を減らす
      std::for_each(signt_ready_queue_.begin(), signt_ready_queue_.end(),
                    [delta_time](float& x) { x -= delta_time; });

      // 先頭から残り時間を消化したぶん、照準が回復する
      // 先にシグナルするか決める
      bool recover = (signt_ready_queue_.front() <= 0.0f);

      while (signt_ready_queue_.front() <= 0.0f) {
        signt_ready_num_ += 1;

        signt_ready_queue_.pop_front();
        if (signt_ready_queue_.empty()) break;
      }

      // 照準の数を送信
      if (recover) {
        Signal::Params params;
        params.insert(Signal::Params::value_type("signt_ready_num", signt_ready_num_));
        fw_.signal().sendMessage(Msg::RECOVER_SIGNT, params);
      }
    }
    
    if (!entry_.isExec() && !stiff_ && !to_target_ && !signt_on_planet_.empty()) {
      // 攻撃開始
      startMoveToTarget();
    }

    if (entry_.isExec()) {
      // 登場時の演出
      y_pos_ += entry_(delta_time);

      if (!entry_.isExec()) {
        stiff_      = true;
        stiff_time_ = params_.at("entry_stiff").get<double>();

        // 着地の振動
        quake_landing_.start(quake_);
        // SE
        gamesound::play(fw_, "player_landing");
      }
    }
    else if (to_target_) {
      // 攻撃移動
      // 回転を更新
      rotate_ = target_rotate_(delta_time);

      // 向きは別に掛け合わせる
      float t = target_(delta_time);
      Quatf q_yaw(Eigen::AngleAxisf(target_yaw_ * t, Vec3f::UnitY()));
      rotate_ = rotate_ * q_yaw;
      
      // x[0, 1.0]で、最大hの高さに到達する放物線は
      // y = -h/0.25 * x^2 + h/0.25 * x で求まる
      y_pos_ += -target_height_ / 0.25f * t * t + target_height_ / 0.25f * t;

      if (!target_.isExec()) {
        // 惑星の着地
        to_target_ = false;
        landing_   = true;
      }
    }
    else if (stiff_) {
      stiff_time_ -= delta_time;
      if (stiff_time_ <= 0.0f) {
        stiff_ = false;
      }
    }
    else {
      // 前進
      float distance = speed_ * delta_time;
      float r = angleOnCircle(distance, planet_radius_);
      Quatf q(Eigen::AngleAxisf(r, Vec3f::UnitX()));
      rotate_ = rotate_ * q;

      // 移動中の振動
      if (!quake_.isExec()) {
        if (randomValue() < quake_move_rate_) {
          quake_move_.start(quake_);
        }
      }
    }

    // 位置情報の更新
    pos_   = rotate_ * Vec3f::UnitY();
    angle_ = rotate_ * Vec3f::UnitX();

    // 攻撃移動から惑星に着地
    if (landing_) {
      landingOnPlanet();
    }

    // アイテム効果
    if (attack_max_power_) {
      attack_max_power_time_ -= delta_time;
      attack_max_power_ = (attack_max_power_time_ > 0.0f);
    }
    if (attack_max_range_) {
      attack_max_range_time_ -= delta_time;
      attack_max_range_ = (attack_max_range_time_ > 0.0f);
    }
    itemEffect(delta_time);

    // 表情
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
    // 強制アイテム発動
    forceItem();
#endif
  }

  // 描画
  void draw(const Signal::Params& arguments) {
    // いきなり描画が呼び出された場合には処理しないための措置
    if (!updated_) return;
    
    modelDraw(model_, model_matrix_.matrix(), *shader_, true);
    shadow_.draw(fw_, shadow_matrix_.matrix());
  }


  // 惑星上のタップ地点に狙いを定める
  void targetOnPlanet(Signal::Params& arguments) {
    // 一回は更新されないと処理しない
    // 照準の残りが無い場合も処理しない
    if (!updated_ || !signt_ready_num_) return;
    
    const Vec3f& on_pos = boost::any_cast<const Vec3f&>(arguments.at("target_pos"));
    const Vec3f& last_pos = getLastTargetPos();

    // 最後の目標 or 自分の位置と、タップした位置が一致した場合は入力を無視
    Vec3f n(on_pos.cross(last_pos));
    if (n.norm() < 0.01f) {
      DOUT << "Oh! last_pos" << std::endl;
      return;
    }
    n = on_pos.cross(pos_);
    if (n.norm() < 0.01f) {
      DOUT << "Oh! pos_" << std::endl;
      return;
    }
    
    // 使用可能な照準を１つ減らす
    signt_ready_num_ -= 1;

    SigntInfo info = {
      boost::any_cast<u_int>(arguments.at("target_hash")),
      on_pos
    };
    signt_on_planet_.push_back(info);

    // 照準を生成
    arguments.insert(Signal::Params::value_type("signt_ready_num", signt_ready_num_));
    fw_.signal().sendMessage(Msg::SPAWN_SIGNT, arguments);
  }

  // 目標への移動開始
  void startMoveToTarget() {
    to_target_ = true;

    // 照準の配列の先頭のが対象になる
    const SigntInfo& signt = signt_on_planet_.front();
    target_hash_ = signt.hash;
    target_pos_  = signt.pos;

    // 移動開始したら照準の配列から取り除く
    signt_on_planet_.pop_front();

    // 目標までの角度
    float angle = angleFromVecs(pos_, target_pos_);
    // 到達時間
    float duration = angle / angleOnCircle(attack_speed_, planet_radius_);
    
    // 回転
    Quatf r(Quatf::FromTwoVectors(pos_, target_pos_));
    target_rotate_.start(params_.at("attack_type").get<std::string>(),
                         duration,
                         rotate_, r * rotate_);

    // 自分の位置と目標の位置の外積と、移動ベクトルを一致させるクオータニオンを求める
    Vec3f to_vec(pos_.cross(target_pos_));
    target_yaw_ = angleFromVecs(angle_, to_vec);

    // 目標の位置と移動ベクトルの内積で右回りか左回りか判別できる
    if (angle_.dot(target_pos_) < 0.0f) target_yaw_ = -target_yaw_;

    // 角度差から、到達時間を求める
    target_.start(params_.at("attack_type").get<std::string>(),
                  duration,
                  0.0f, 1.0f);

    // 角度差から、到達高度を決める
    target_height_ = attack_height_ * angle / m_pi;

    // 攻撃力[0, 1]
    float dist = distOnCircle(angle, planet_radius_);
    target_power_ = std::min(dist / attack_distance_, 1.0f);

    // アイテム効果で常に最大
    if (attack_max_range_) target_power_ = 1.0f;

    // SE
    gamesound::play(fw_, "player_jump");
  }

  // 着地
  void landingOnPlanet() {
    landing_ = false;
    in_game_.attack_num += 1;
    
    {
      // 接触判定
      Signal::Params params;
      params.insert(Signal::Params::value_type("pos", pos_));

      // Cubeの大きさを考慮した攻撃範囲
      float dist = attack_range_(0) + (attack_range_(1) - attack_range_(0)) * target_power_ * target_power_;
      dist += 1.4f * radius_;
      params.insert(Signal::Params::value_type("dist", dist));
      params.insert(Signal::Params::value_type("angle", angleOnCircle(dist, planet_radius_)));
      params.insert(Signal::Params::value_type("power", attack_max_power_ ? 100 : 1));

      // 画面振動用
      params.insert(Signal::Params::value_type("target_hash", target_hash_));
      params.insert(Signal::Params::value_type("speed", 22.0f));
      params.insert(Signal::Params::value_type("max_power", attack_max_power_));

      // SE
      static const std::string touchdown[] = {
        "touchdown4",
        "touchdown3",
        "touchdown2",
        "touchdown1"
      };
      int index = minmax(static_cast<int>(target_power_ * 4.0f + 0.5f), 0, 3);
      gamesound::play(fw_, touchdown[index]);

      // 表情
      model_.materialTexture(face_attack_);
      face_change_time_ = static_cast<float>(params_.at("face_attack_time").get<double>());

      DOUT << "target_power:" << target_power_ << " Dist" << dist << " SE:" << index << std::endl;

      // プレイヤーの攻撃をシグナル
      fw_.signal().sendMessage(Msg::TOUCHDOWN_PLANET, params);
      
      if (params.find("hit") != params.cend()) {
        // 攻撃成功回数をカウント
        in_game_.attack_hit_num += 1;
        in_game_.attack_combo_num += 1;
      }
      else {
        // HITが無ければ連続成功数をクリア
        in_game_.attack_combo_max = std::max(in_game_.attack_combo_num, in_game_.attack_combo_max);
        in_game_.attack_combo_num = 0;
      }
      
      // 連続攻撃成功数
      params.insert(Signal::Params::value_type("hit_combo", in_game_.attack_combo_num));

      const auto it = params.find("destroy");
      if (it != params.cend()) {
        // 敵を破壊した
        // int destroy_num = boost::any_cast<int>(it->second);
        // DOUT << "Enemy destroy:" << destroy_num << std::endl;
        
        fw_.signal().sendMessage(Msg::DESTROYED_ENEMY, params);
      }
      else {
        // 撃破無しだが、攻撃はHIT
        fw_.signal().sendMessage(Msg::ATTACK_HIT_ENEMY, params);
      }

      // 本体振動
      quake_attack_.start(quake_);
    }
    
    // 照準の回復待ち開始
    signt_ready_queue_.push_back(signt_ready_time_);

    // 着地直後の硬直
    stiff_      = true;
    stiff_time_ = params_.at("attack_stiff").get<double>();
  }
  
  // 最後の目標座標を取得
  // 目標が無い場合は自分の位置を返す
  const Vec3f& getLastTargetPos() const {
    if (!signt_on_planet_.empty()) return signt_on_planet_.back().pos;
    return to_target_ ? target_pos_ : pos_;
  }
  
  // 生成情報を元に初期化
  void setupFromSpawnInfo(const Signal::Params& arguments) {
    planet_radius_ = boost::any_cast<float>(arguments.at("planet_radius"));
    rotate_        = boost::any_cast<Quatf>(arguments.at("spawn_rotate"));

    pos_   = rotate_ * Vec3f::UnitY();
    angle_ = rotate_ * Vec3f::UnitX();

    shadow_.setup(radius_ + params_.at("shadow_radius").get<double>(), planet_radius_, 0.04f);
  }


  // 情報を返す
  void objectInfo(Signal::Params& arguments) {
    if (!updated_) return;

    Msg::PlayerInfo info = {
      this,
      hash_, !to_target_,
      pos_, angle_, radius_,
      to_target_
    };
    arguments.insert(Signal::Params::value_type("player_info", info));
  }
  
  // 相互干渉
  void interference(const Signal::Params& arguments) {
    if (!updated_) return;

    if (!Signal::isParamValue(arguments, "enemy_info")) return;

    const auto& infos = boost::any_cast<const std::deque<Msg::EnemyInfo>&>(arguments.at("enemy_info"));
    for (const auto& info : infos) {
      if (!info.collision) continue;

      // 接触判定(矩形がすっぽりおさまる円で判定)
      Vec3f pv(pos_.cross(info.pos));
      float angle = angleFromVecs(pos_, info.pos);
      float dist  = distOnCircle(angle, planet_radius_);
      float d_min = (radius_ + info.radius) * 1.4;
      if (dist < d_min) {
        // 相手も接触判定するので、半分だけ移動
        float d = (d_min - dist) / 2.0f;
        Quatf q(Eigen::AngleAxisf(-angleOnCircle(d, planet_radius_), pv.normalized()));
        rotate_ = q * rotate_;
      }
    }    
  }

  // Baseとの接触判定
  void hitCheckWithBase(Signal::Params& arguments) {
    // 相手のローカル座標に変換したSphereを用意
    Eigen::Affine3f matrix =
      boost::any_cast<Eigen::Affine3f&>(arguments.at("rev_matrix")) * model_matrix_;
    
    SphereVolume l_volume = {
      matrix * Vec3f(0.0f, 0.5f, 0.0f),
      (radius_ * 0.8f)/ boost::any_cast<float>(arguments.at("scale"))
    };

    bool contact = testSphereAABB(l_volume, boost::any_cast<AABBVolume&>(arguments.at("volume")));
    arguments.insert(Signal::Params::value_type("contact", contact));
  }
  
  // アイテムを拾う処理
  void pickUpItem(Signal::Params& arguments) {
    // 地上から離れている場合は取れない
    if (y_pos_ > (planet_radius_ + radius_ * 2.0f)) return;

    // 効果演出
    item_effect_       = true;
    item_effect_time_  = boost::any_cast<float>(arguments.at("effect_time"));
    item_effect_color_ = boost::any_cast<Vec3f>(arguments.at("item_color"));
    item_effect_ease_  = boost::any_cast<Ease<float>&>(arguments.at("effect_ease"));
    // SE
    gamesound::play(fw_, "item_get");
    // 実績
    gamecenter::achievement("NGS0004UIROU.item", 100.0, demo_mode_);
    
    const auto item_type = boost::any_cast<int>(arguments.at("item_type"));
    switch (item_type) {
    case 0:
      // 一定時間攻撃力MAX
      DOUT << "MAX ATTACK POWER." << std::endl;
      fw_.signal().sendMessage(Msg::ITEM_MAX_POWER, arguments);
      break;

    case 1:
      // 一定時間敵硬直
      DOUT << "ENEMY STIFF" << std::endl;
      fw_.signal().sendMessage(Msg::ITEM_ENEMY_STIFF, arguments);
      break;

    case 2:
      // 一定時間白ういろう無敵
      DOUT << "BASE IMMORTAL" << std::endl;
      fw_.signal().sendMessage(Msg::ITEM_BASE_IMMORTAL, arguments);
      break;

    case 3:
      // 一定時間スコア倍増
      DOUT << "SCORE X4" << std::endl;
      fw_.signal().sendMessage(Msg::ITEM_SCORE_MULTIPLY, arguments);
      break;

    case 4:
      // 一定時間攻撃範囲MAX
      DOUT << "MAX ATTACK RANGE" << std::endl;
      fw_.signal().sendMessage(Msg::ITEM_MAX_RANGE, arguments);
      break;

    default:
      assert(0);
    }

    // アイテムに「拾った」と伝える
    arguments.insert(Signal::Params::value_type("get", true));
  }

  
  // アイテム効果: 最大攻撃力
  void itemMaxPower(const Signal::Params& arguments) {
    attack_max_power_      = true;
    attack_max_power_time_ = boost::any_cast<float>(arguments.at("effect_time"));
  }

  // アイテム効果: 最大攻撃範囲
  void itemMaxRange(const Signal::Params& arguments) {
    attack_max_range_      = true;
    attack_max_range_time_ = boost::any_cast<float>(arguments.at("effect_time"));
  }

  // アイテム演出
  void itemEffect(const float delta_time) {
    if (!item_effect_) return;

    item_effect_time_ -= delta_time;
    if (item_effect_time_ > 0.0f) {
      float color_ease   = item_effect_ease_(delta_time);
      Vec3f effect_color = item_effect_color_ * color_ease;

      // エミッシブを書き換える
      std::deque<Material>& model_materials = model_.material();
      for (u_int i = 0; i < materials_.size(); ++i) {
        Vec3f col = materials_[i].emissive() * (1.0f - color_ease) + effect_color;
        model_materials[i].emissive(col);
      }
    }
    else {
      item_effect_ = false;

      // エミッシブを元に戻す
      std::deque<Material>& model_materials = model_.material();
      for (u_int i = 0; i < materials_.size(); ++i) {
        model_materials[i].emissive(materials_[i].emissive());
      }
    }
  }

  // ゲーム本編開始
  void startGameMain() {
    // 各種記録を初期化
    in_game_.attack_num       = 0;
    in_game_.attack_hit_num   = 0;
    in_game_.attack_combo_num = 0;
    in_game_.attack_combo_max = 0;
  }

  // 記録を収集
  void gatherGameResult(Signal::Params& arguments) {
    arguments.insert(Signal::Params::value_type("attack_num", results_.attack_num));
    arguments.insert(Signal::Params::value_type("attack_hit_num", results_.attack_hit_num));
    arguments.insert(Signal::Params::value_type("attack_combo_num", results_.attack_combo_num));
    arguments.insert(Signal::Params::value_type("attack_combo_max", results_.attack_combo_max));
  }


  // 操作の記録・再生
  void execDemoMode(const Signal::Params& params) {
    demo_mode_ = true;
    quake_.stop();
  }

  void recordInfo(Signal::Params& params) {
    params.insert(Signal::Params::value_type("rotate", rotate_));
  }

  void playbackInfo(Signal::Params& params) {
    rotate_ = boost::any_cast<Quatf>(params.at("rotate"));
    
    pos_   = rotate_ * Vec3f::UnitY();
    angle_ = rotate_ * Vec3f::UnitX();
  }

  
#ifdef _DEBUG
  // 強制アイテム発動
  void forceItem() {
    bool  exec        = false;
    int   msg;
    int   type;

    char key = fw_.keyboard().getPushed();
    if (key == '1') {
      DOUT << "MAX ATTACK POWER." << std::endl;

      exec = true;
      type = 0;
      msg  = Msg::ITEM_MAX_POWER;
    }
    else if (key == '2') {
      DOUT << "ENEMY STIFF" << std::endl;

      exec = true;
      type = 1;
      msg  = Msg::ITEM_ENEMY_STIFF;
    }
    else if (key == '3') {
      DOUT << "BASE IMMORTAL" << std::endl;

      exec = true;
      type = 2;
      msg  = Msg::ITEM_BASE_IMMORTAL;
    }
    else if (key == '4') {
      DOUT << "SCORE X4" << std::endl;

      exec = true;
      type = 3;
      msg  = Msg::ITEM_SCORE_MULTIPLY;
    }
    else if (key == '5') {
      DOUT << "MAX ATTACK RANGE" << std::endl;

      exec = true;
      type = 4;
      msg  = Msg::ITEM_MAX_RANGE;
    }

    if (exec) {
      const picojson::value& item_params = json_.at("cubeItem");
      const picojson::array& colors(item_params.at("color").get<picojson::array>());

      item_effect_       = true;
      item_effect_time_  = 10.0f;
      item_effect_ease_  = easeFromJson<float>(item_params.at("effect_ease"));
      item_effect_color_ = vectFromJson<Vec3f>(colors[type]);
      
      Signal::Params params;
      params.insert(Signal::Params::value_type("effect_time", item_effect_time_));
      fw_.signal().sendMessage(msg, params);
    }
  }

#endif
  
};

}
