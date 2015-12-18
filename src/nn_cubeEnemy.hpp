
#pragma once

//
// 敵CUBE
//

#include "co_defines.hpp"
#include <memory>
#include <Eigen/Geometry>
#include "co_easyShader.hpp"
#include "co_modelDraw.hpp"
#include "co_random.hpp"
#include "co_easing.hpp"
#include "co_miniEasing.hpp"
#include "co_quatEasing.hpp"
#include "co_misc.hpp"
#include "nn_modelHolder.hpp"
#include "nn_shaderHolder.hpp"
#include "nn_messages.hpp"
#include "nn_objBase.hpp"
#include "nn_cpuFactory.hpp"
#include "co_miniQuake.hpp"
#include "co_quakeParam.hpp"


namespace ngs {

class CubeEnemy : public ObjBase {
  Framework& fw_;
  const picojson::value& params_;
  const CpuFactory& cpu_factory_;

  bool active_;
  bool updated_;
  bool pause_;

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

  // 最大旋回力
  float yaw_max_;
  
  // 体力
  int hp_max_;
  int hp_;

  // 位置ベクトル
  Vec3f pos_;
  // 回転ベクトル
  Vec3f angle_;
  
  // 表示用行列
  Eigen::Affine3f model_matrix_;

  // 目標の有無
  bool target_;

  // 退去
  bool        leave_;
  Ease<float> leave_scale_;

  MiniEasing<float> y_move_;

  bool  stiff_;
  float stiff_time_;

  bool  avoid_;
  float avoid_turn_;
  float avoid_speed_;

  // 破壊された時の演出
  QuatEasing destroy_;

  // 登場演出
  MiniEasing<float> appear_;
  
  // 消滅演出
  MiniEasing<float> disappear_;
  float             disappear_scale_;
  
  // 消滅時にアイテム生成
  bool spawn_item_;

  // ジャンプ移動
  bool  jumping_;
  float jump_speed_;
  Vec3f jump_pos_;
  float jump_to_yaw_;
  float jump_to_height_;

  MiniEasing<float> jump_;
  QuatEasing jump_rotate_;

  // ダメージ演出
  bool        damaged_;
  Ease<Vec3f> damaged_color_;
  Vec3f       wounded_color_;

  // CPU
  std::unique_ptr<Cpu> cpu_;

  // アイテム効果
  bool  force_stiff_;
  float force_stiff_time_;
  
  // 振動
  MiniQuake  quake_;
  QuakeParam quake_landing_;
  QuakeParam quake_damage_;
  QuakeParam quake_move_;
  float      quake_move_rate_;
  
  // 影
  CubeShadow      shadow_;
  Eigen::Affine3f shadow_matrix_;
  GrpCol          shadow_color_;

  
public:
  CubeEnemy(Framework& fw,
            const picojson::value& params,
            const std::string& name,
            CpuFactory& cpu_factory,
            ModelHolder& model_holder, ShaderHolder& shader_holder,
            const CubeShadow& shadow) :
    fw_(fw),
    params_(params.at(name)),
    cpu_factory_(cpu_factory),
    active_(true),
    updated_(false),
    pause_(false),
    hash_(createUniqueNumber()),
    model_(model_holder.read(params_.at("model").get<std::string>())),
    // materials_(model_.material()),
    shader_(shader_holder.read(params_.at("shader").get<std::string>())),
    radius_(randomValue(vectFromJson<Vec2f>(params_.at("radius")))),
    scale_(radius_ * 2.0f),
    y_pos_(200.0f),
    rotate_(Eigen::AngleAxisf(randomValue() * m_pi, randomVector<Vec3f>())),
    yaw_(0.0f),
    speed_(randomValue(vectFromJson<Vec2f>(params_.at("speed")))),
    yaw_max_(deg2rad(randomValue(vectFromJson<Vec2f>(params_.at("yaw_max"))))),
    hp_max_(params_.at("HP").get<double>()),
    hp_(hp_max_),
    model_matrix_(Eigen::Affine3f::Identity()),
    target_(false),
    leave_(false),
    leave_scale_(easeFromJson<float>(params_.at("leave_scale"))),
    y_move_(miniEasingFromJson<float>(params_.at("entry"))),
    stiff_(false),
    avoid_(false),
    appear_(miniEasingFromJson<float>(params_.at("appear"))),
    disappear_(miniEasingFromJson<float>(params_.at("disappear"))),
    disappear_scale_(1.0f),
    spawn_item_(false),
    jumping_(false),
    jump_speed_(randomValue(vectFromJson<Vec2f>(params_.at("jump_speed")))),
    damaged_(false),
    damaged_color_(easeFromJson<Vec3f>(params_.at("damaged_color"))),
    wounded_color_(vectFromJson<Vec3f>(params_.at("wounded_color"))),
    cpu_(cpu_factory(params_.at("start_cpu").get<std::string>(), params_.at("start_cpu_param"))),
    force_stiff_(false),
    quake_landing_(params_.at("quake_landing")),
    quake_damage_(params_.at("quake_damage")),
    quake_move_(params_.at("quake_move")),
    quake_move_rate_(params_.at("quake_move_rate").get<double>()),
    shadow_(shadow),
    shadow_matrix_(Eigen::Affine3f::Identity()),
    shadow_color_(vectFromJson<GrpCol>(params_.at("shadow_color")))
  {
    DOUT << "CubeEnemy()" << std::endl;

    // 表情テクスチャ
    model_.materialTexture(fw_.loadPath() + params_.at("texture").get<std::string>());

    // FIXME:マテリアルのdeffuseだけ書き換える(colladaがテクスチャ付きのに対応していない)
    Model::materialDiffuseColor(model_, vectFromJson<Vec3f>(params_.at("material_deffuse")));
    materials_ = model_.material();

    // 消滅演出を止めておく
    disappear_.stop();
  }

  ~CubeEnemy() {
    DOUT << "~CubeEnemy()" << std::endl;
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

    case Msg::ENEMY_SPAWN_ITEM:
      spawn_item_ = true;
      return;

    case Msg::CHECK_HIT_BASE:
      hitCheckWithBase(arguments);
      return;

    case Msg::DESTROYED_BASE:
      leavePlanet();
      return;

    case Msg::ITEM_ENEMY_STIFF:
      itemEnemyStiff(arguments);
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
    
    // いきなり描画が呼び出された場合には処理しないための措置
    updated_ = true;
    
    float delta_time = boost::any_cast<float>(arguments.at("delta_time"));

    y_pos_ = planet_radius_ - 0.5f;

    if (stiff_) {
      // 硬直
      stiff_time_ -= delta_time;
      if (stiff_time_ <= 0.0f) stiff_ = false;
    }
    else if (y_move_.isExec()) {
      // 登場 or 撤収時のふわっと動く演出
      y_pos_ += y_move_(delta_time);
      if (!y_move_.isExec()) {
        if (leave_) {
          // 消滅
          active_ = false;
          return;
        }
        else {
          // 着地
          stiff_      = true;
          stiff_time_ = params_.at("landing_stiff").get<double>();

          // 着地の振動
          quake_landing_.start(quake_);
          // SE
          gamesound::play(fw_, "enemy_landing");
        }
      }
    }
    else if (disappear_.isExec()) {
      effectDisappear(delta_time);
    }
    else if (destroy_.isExec()) {
      // 破壊された場合
      moveDestroyed(delta_time);
    }
    else if (avoid_) {
      // 回避行動中
      avoidOtherCube(delta_time);
      cpu_->idle(delta_time);
    }
    else if (jumping_) {
      // ジャンプ移動中
      jumpToTarget(delta_time);
      cpu_->idle(delta_time);
    }
    else {
      // CPUによる自立行動
      cpuAction(delta_time);
    }

    // ダメージ演出
    if (damaged_) {
      Vec3f color = damaged_color_(delta_time);
      damaged_ = !damaged_color_.isEnd();
      Model::materialEmissiveColor(model_, damaged_ ? color : Vec3f::Zero());
    }
    
    // 位置情報の更新
    pos_   = rotate_ * Vec3f::UnitY();
    angle_ = rotate_ * Vec3f::UnitX();

    // アイテム効果
    if (force_stiff_) {
      force_stiff_time_ -= delta_time;
      force_stiff_ = (force_stiff_time_ > 0.0f);
    }

    // 振動効果
    float quake_scale = quake_(delta_time) * scale_;

    float scale = scale_;
    if (appear_.isExec()) {
      // 登場時
      scale *= appear_(delta_time);
    }
    else if(disappear_.isExec()) {
      scale *= disappear_scale_;
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
    float d = (50.0f - minmax(y_pos_ - planet_radius_, 0.0f, 50.0f)) / 50.0f;
    shadow_.color(shadow_color_ * d * disappear_scale_);
  }

  // 描画
  void draw(const Signal::Params& arguments) {
    // いきなり描画が呼び出された場合には処理しないための措置
    if (!updated_) return;
    
    modelDraw(model_, model_matrix_.matrix(), *shader_, true);
    shadow_.draw(fw_, shadow_matrix_.matrix());
  }

  // 消滅演出
  void effectDisappear(const float delta_time) {
    // 消滅演出
    disappear_scale_ = disappear_(delta_time);
    if (!disappear_.isExec()) {
      active_ = false;

      // cubeItem生成
      if (spawn_item_) {
        Signal::Params params;
        params.insert(Signal::Params::value_type("spawn_pos", pos_));
        
        fw_.signal().sendMessage(Msg::SPAWN_ITEM, params);
      }
    }
  }

  // 破壊された時の動き
  void moveDestroyed(const float delta_time) {
    rotate_ = destroy_(delta_time);
    if (!destroy_.isExec()) {
      if (hp_ > 0) {
        // まだ体力があった!!
        return;
      }
      else {
        // 消滅演出開始
        disappear_.resume();
      }
    }
  }

  // CPUによる自立行動
  void cpuAction(const float delta_time) {
    // アイテム効果
    if (force_stiff_) return;

    // 自分の状況をCPUに渡して次の行動を決める
    Cpu::Self self = {
      pos_,
      angle_,
      rotate_,
      target_,
      float(hp_) / hp_max_
    };
    Cpu::Action action = cpu_->update(delta_time, self);
      
    // 旋回
    float yaw = minmax(action.yaw, -yaw_max_, yaw_max_);
    Quatf q(Eigen::AngleAxisf(yaw * delta_time, Vec3f::UnitY()));
    rotate_ = rotate_ * q;

    // 前進
    moveForward(speed_ * action.acc * delta_time);

    // ジャンプ
    if (action.jump) {
      startJumpToTarget(action.jump_pos);
    }

    // 白ういろう再検索
    target_ = !action.re_target;
  }

  
#if 0
  // 目標目指して進む
  void moveToTarget(const float delta_time) {
    if (!target_) {
      // 目標を探す
      target_seach_time_ -= delta_time;
      if (target_seach_time_ <= 0.0f) {
        // 目標検索
        target_ = getBasePosition();
        if (!target_) {
          // 見つからなければ一定時間ごとに探す(シグナルは重たい)
          target_seach_time_ = randomValue(1.0f, 2.0f);
        }
      }
    }
    else {
      // 自分の位置と目標の位置の外積と、移動ベクトルが一致すれば
      // 目標へ到達できる
      float r = angleFromVecs(angle_, pos_.cross(target_pos_));

      // 目標の位置と移動ベクトルの内積で右回りか左回りか判別できる
      if (angle_.dot(target_pos_) < 0.0f) r = -r;

      // 向きを [-yaw_max, yaw_max] に収める
      float yaw_max = deg2rad(float(params_.at("yaw_max").get<double>()));
      r = minmax(r, -yaw_max, yaw_max);
      
      // 旋回
      Quatf q(Eigen::AngleAxisf(r * delta_time, Vec3f::UnitY()));
      rotate_ = rotate_ * q;

#if 0
      // 左右旋回はこう書く
      if (fw_.keyboard().isPress(Keyboard::LEFT)) {
        Quatf q(Eigen::AngleAxisf(m_pi / 50.0f, Vec3f::UnitY()));
        rotate_ = rotate_ * q;
      }
      else if (fw_.keyboard().isPress(Keyboard::RIGHT)) {
        Quatf q(Eigen::AngleAxisf(-m_pi / 50.0f, Vec3f::UnitY()));
        rotate_ = rotate_ * q;
      }
    
      if (fw_.keyboard().isPress(Keyboard::UP)) {
        Quatf q(Eigen::AngleAxisf(m_pi / 200.0f, Vec3f::UnitX()));
        rotate_ = rotate_ * q;
      }
      else if (fw_.keyboard().isPress(Keyboard::DOWN)) {
        Quatf q(Eigen::AngleAxisf(-m_pi / 200.0f, Vec3f::UnitX()));
        rotate_ = rotate_ * q;
      }
#endif

    }

    // 前進
    moveForward(speed_ * delta_time);
  }
#endif

  
  // 目標へジャンプして移動開始
  void startJumpToTarget(const Vec3f& pos) {
    jumping_  = true;
    jump_pos_ = pos;

    // 目標までの角度
    float angle = angleFromVecs(pos_, jump_pos_);
    // 角度差から、到達時間を求める
    float duration =
      angle / angleOnCircle(jump_speed_, planet_radius_);
    
    // 着地時のクオータニオンを求める
    Quatf r(Quatf::FromTwoVectors(pos_, jump_pos_));
    jump_rotate_.start(params_.at("jump_type").get<std::string>(),
                       duration,
                       rotate_, r * rotate_);

    // 自分の位置と目標の位置の外積と、移動ベクトルを一致させるクオータニオンを求める
    Vec3f to_vec(pos_.cross(jump_pos_));
    jump_to_yaw_ = angleFromVecs(angle_, to_vec);

    // 目標の位置と移動ベクトルの内積で右回りか左回りか判別できる
    if (angle_.dot(jump_pos_) < 0.0f) jump_to_yaw_ = -jump_to_yaw_;

    jump_.start(params_.at("jump_type").get<std::string>(),
                duration,
                0.0f, 1.0f);

    // 角度差から、到達高度を決める
    jump_to_height_ =
      static_cast<float>(params_.at("jump_height").get<double>()) * angle / m_pi;

    // SE
    gamesound::play(fw_, "enemy_jump");
  }
  
  // 目標へジャンプして移動
  void jumpToTarget(const float delta_time) {
    // 回転を更新
    rotate_ = jump_rotate_(delta_time);

    // 向きは別に掛け合わせる
    float t = jump_(delta_time);
    Quatf q_yaw(Eigen::AngleAxisf(jump_to_yaw_ * t, Vec3f::UnitY()));
    rotate_ = rotate_ * q_yaw;
      
    // x[0, 1.0]で、最大hの高さに到達する放物線は
    // y = -h/0.25 * x^2 + h/0.25 * x で求まる
    y_pos_ += -jump_to_height_ / 0.25f * t * t + jump_to_height_ / 0.25f * t;

    if (!jump_.isExec()) {
      // 惑星に着地
      jumping_    = false;
      stiff_      = true;
      stiff_time_ = params_.at("jump_stiff").get<double>();

      // 着地の振動
      quake_landing_.start(quake_);

      // SE
      gamesound::play(fw_, "enemy_landing");
    }
  }
  

  // 回避行動
  void avoidOtherCube(const float delta_time) {
    // アイテム効果
    if (force_stiff_) return;
    
    // 旋回
    Quatf q(Eigen::AngleAxisf(avoid_turn_ * delta_time, Vec3f::UnitY()));
    rotate_ = rotate_ * q;
   
    // 前進
    moveForward(speed_ * avoid_speed_ * delta_time);
  }


  // 前進するクオータニオンを計算
  void moveForward(const float move_dist) {
    float move_angle = angleOnCircle(move_dist, planet_radius_);
    Quatf q(Eigen::AngleAxisf(move_angle, Vec3f::UnitX()));
    rotate_ = rotate_ * q;

    // 移動中の振動
    if (!quake_.isExec()) {
      if (randomValue() < quake_move_rate_) {
        quake_move_.start(quake_);
      }
    }
  }


  // Baseとの接触判定
  void hitCheckWithBase(Signal::Params& arguments) {
    // TIPS:オウンゴールを認める
    // if (destroy_.isExec()) return;

    // 角度差からざっくり判定
    float angle = angleFromVecs(pos_, boost::any_cast<Vec3f>(arguments.at("pos")));
    float dist = distOnCircle(angle, planet_radius_);
    if (dist > ((radius_ + boost::any_cast<float>(arguments.at("radius"))) * 2.0f)) return;

#if 0
    // ざっくり判定確認コード
    if (dist < ((radius_ + boost::any_cast<float>(arguments.at("radius"))) * 2.0f)) {
      Signal::modifyParamValue(arguments, "hit_num", 1);
      active_ = false;
      return;
    }
#endif
    
    // 相手のローカル座標に変換したSphereを用意
    Eigen::Affine3f matrix =
      boost::any_cast<Eigen::Affine3f&>(arguments.at("rev_matrix")) * model_matrix_;
    
    SphereVolume l_volume = {
      matrix * Vec3f(0.0f, 0.5f, 0.0f),
      radius_ / boost::any_cast<float>(arguments.at("scale"))
    };

    if (testSphereAABB(l_volume, boost::any_cast<AABBVolume&>(arguments.at("volume")))) {
      // 接触した事をシグナル元に伝える
      Signal::modifyParamValue(arguments, "hit_num", 1);
      active_ = false;
    }
  }

  // Playerの攻撃との接触判定
  void hitCheckWithPlayer(Signal::Params& arguments) {
    bool collision = abs(y_pos_ - planet_radius_) < 1.5f;
    if (!collision || leave_ || destroy_.isExec() || disappear_.isExec()) return;
    // if (y_move_.isExec() || destroy_.isExec()) return;

    // TIPS:Cubeの大きさを考慮した角度差
    const Vec3f& pos = boost::any_cast<Vec3f>(arguments.at("pos"));
    float angle = angleFromVecs(pos, pos_);
    angle -= angleOnCircle(1.4f * radius_, planet_radius_);

    if (angle < boost::any_cast<float>(arguments.at("angle"))) {
      // 攻撃HIT
      Signal::modifyParamValue(arguments, "hit", 1);
      
      hp_ -= boost::any_cast<int>(arguments.at("power"));
      if (hp_ <= 0) {
        // 破壊された
        hp_ = 0;
        Signal::modifyParamValue(arguments, "destroy", 1);

        Msg::EnemyInfo info = {
          this, hash_,
          false,
          pos_, angle_, radius_,
          false
        };
        Signal::pushbackParamValue(arguments, "destroy_enemy", info);
      }
      else {
        // HPが減ったら赤くする
        float rate = 1.0f - float(hp_) / hp_max_;
        modifyDiffuseColor(model_, materials_, wounded_color_ * rate);
      }
      destroyAction(pos);
    }
  }

  // 破壊された時の行動を決める
  void destroyAction(const Vec3f& pos) {
    // ぶっとび後のクオータニオン
    Quatf rotate_end =
      Eigen::AngleAxisf(-angleOnCircle(radius_ * 4, planet_radius_), pos_.cross(pos).normalized()) * rotate_;

    // ぶっとび演出開始
    destroy_.start(params_.at("destroy_type").get<std::string>(),
                   params_.at("destroy_duration").get<double>(),
                   rotate_, rotate_end);

    y_move_.stop();

    // 振動
    quake_damage_.start(quake_);

    // 着地時の硬直を解除
    stiff_ = false;

    damaged_ = true;
    damaged_color_.toStart();
  }


  // 去る演出開始
  void leavePlanet() {
    target_ = false;
    leave_  = true;
    miniEasingFromJson<float>(y_move_, params_.at("leave"));
  }
  
  
  // 情報を返す
  void objectInfo(Signal::Params& arguments) {
    if (!updated_) return;

    bool collision = !y_move_.isExec();
    Msg::EnemyInfo info = {
      this, hash_,
      collision,
      pos_, angle_, radius_,
      jumping_
    };
    Signal::pushbackParamValue(arguments, "enemy_info", info);
  }

  // 相互干渉
  void interference(const Signal::Params& arguments) {
    if (!updated_) return;

    if (!target_) {
      target_ = searchBase(arguments);
    }

    avoidOtherObjects(arguments);
  }

  // 白uirouを探してCPUに伝える
  bool searchBase(const Signal::Params& arguments) {
    if (!Signal::isParamValue(arguments, "base_info")) return false;

    const auto& infos = boost::any_cast<const std::deque<Msg::BaseInfo>&>(arguments.at("base_info"));

    // 白ういろうが複数いる場合はランダムで
    const auto& info = infos[randomValue(static_cast<int>(infos.size()))];

    // CPUに情報を伝える
    Cpu::Target target = {
      info.pos,
      info.angle,      
      info.rotate,
      planet_radius_
    };
    cpu_->target(target);

    return true;
  }
  
  // オブジェクト同士ぶつからないようにする
  void avoidOtherObjects(const Signal::Params& arguments) {
    if (!updated_) return;
    
    avoid_ = false;

    // jump中、上下移動中は判定しない
    if (jumping_ || y_move_.isExec()) return;

    float dist_near = planet_radius_;
    // プレイヤーとの判定
    if (Signal::isParamValue(arguments, "player_info")) {
      const auto& info = boost::any_cast<const Msg::PlayerInfo&>(arguments.at("player_info"));
      avoidObject(dist_near, info.pos, info.radius, info.collision);
    }

    // 敵同士の判定
    const auto& infos = boost::any_cast<const std::deque<Msg::EnemyInfo>&>(arguments.at("enemy_info"));
    for (const auto& info : infos) {
      // 自分自身との判定はしない
      if (info.hash == hash_) continue;
      avoidObject(dist_near, info.pos, info.radius, info.collision);
    }
  }

  // オブジェクト回避
  void avoidObject(float& dist_near, const Vec3f& pos, const float radius, const bool collision) {
    // 互いが一定以内の距離にあるか判定
    float angle = angleFromVecs(pos_, pos);
    float dist  = distOnCircle(angle, planet_radius_);
    if (dist > (radius_ * 5.0f)) return;

    Vec3f pv(pos_.cross(pos));

    // 接触判定(矩形がすっぽりおさまる円で判定)
    if (collision) {
      float d_min = (radius_ + radius) * 1.4f;
      if (dist < d_min) {
        // 相手も接触判定するので、半分だけ移動
        float d = (d_min - dist) / 2;
        Quatf q(Eigen::AngleAxisf(-angleOnCircle(d, planet_radius_), pv.normalized()));
        rotate_ = q * rotate_;
      }
    }

    // 自分の進行方向と相手への方向の差が45度以内か判定
    float da = angleFromVecs(angle_, pv);
    if (da > (m_pi / 4)) return;
      
    // ループ内で判定した相手より遠いのは判定しない
    if (dist > dist_near) return;
    dist_near = dist;

    // 目標の位置と移動ベクトルの内積で右回りか左回りか判別できる
    float yaw_max = yaw_max_;
    if (angle_.dot(pos) < 0.0f) yaw_max = -yaw_max;
    avoid_       = true;
    avoid_turn_  = -yaw_max;
    avoid_speed_ = (dist < (radius_ * 5.0f)) ? 0.25f : 0.8f;
  }

  
  // 設定値を元に初期設定
  void setupFromSpawnInfo(const Signal::Params& arguments) {
    planet_radius_ = boost::any_cast<float>(arguments.at("planet_radius"));

    // 惑星上の座標から行列生成
    Vec3f pos = boost::any_cast<Vec3f>(arguments.at("spawn_pos"));
    rotate_.setFromTwoVectors(Vec3f::UnitY(), pos);

    // 向きは別に掛け合わせる
    Quatf q(Eigen::AngleAxisf(randomValue(-m_pi, m_pi), Vec3f::UnitY()));
    rotate_ = rotate_ * q;

    // レベルによる性能アップ
    speed_      *= boost::any_cast<float>(arguments.at("speed"));
    yaw_max_    *= boost::any_cast<float>(arguments.at("yaw"));
    jump_speed_ *= boost::any_cast<float>(arguments.at("jump_speed"));

    // アイテム効果
    force_stiff_      = boost::any_cast<bool>(arguments.at("force_stiff"));
    force_stiff_time_ = boost::any_cast<float>(arguments.at("force_stiff_time"));
    
    shadow_.setup(radius_ + params_.at("shadow_radius").get<double>(), planet_radius_, 0.05f);
  }

  // アイテム効果で硬直
  void itemEnemyStiff(const Signal::Params& arguments) {
    force_stiff_      = true;
    force_stiff_time_ = boost::any_cast<float>(arguments.at("effect_time"));
  }


  // マテリアルカラーに色を加える
  static void modifyDiffuseColor(Model& model, const std::deque<Material>& materials_src, const Vec3f& col) {
    std::deque<Material>& materials = model.material();
    for (u_int i = 0; i < materials.size(); ++i) {
      materials[i].diffuse(materials_src[i].diffuse() + col);
    }
  }
  
};

}
