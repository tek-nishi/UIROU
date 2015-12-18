
#pragma once

//
// ルールに基づいて敵を生成する
//

#include "co_defines.hpp"
#include "co_framework.hpp"
#include "co_miniEasing.hpp"
#include "nn_messages.hpp"
#include "nn_objBase.hpp"


namespace ngs {

class Generator : public ObjBase {
  Framework& fw_;
  const picojson::value& params_;
  const picojson::array& patterns_;

  bool active_;
  bool pause_;
  bool updated_;

  // 黒ういろう生成
  bool  setup_;
  bool  spawn_;
  u_int pattern_index_;
  int   entry_level_;

  // 動作ウエイト
  bool  wait_clean_;
  float delay_;

  // 生成数
  int   spawn_num_;
  Vec2i once_spawn_;
  
  Vec2f interval_;
  float current_interval_;
  Vec2f angle_;

  const picojson::array* types_;

  // レベル設定(出現パターンを一巡すると上がる)
  int level_max_;
  int level_;
  
  Vec2f level_speed_;
  Vec2f level_yaw_;
  Vec2f level_jump_speed_;

  // アイテム効果で、生成時に硬直
  bool  stiff_spawn_;
  float stiff_spawn_time_;

  
  // 白ういろう生成
  const picojson::array& base_entry_;

  bool  base_setup_;
  bool  base_spawn_;
  u_int base_spawn_index_;
  int   base_entry_level_;
  Vec2f base_spawn_angle_;

  
public:
  explicit Generator(Framework& fw, const picojson::value& params) :
    fw_(fw),
    params_(params.at("generator")),
    patterns_(params_.at("patterns").get<picojson::array>()),
    active_(true),
    pause_(false),
    updated_(false),
    setup_(true),
    spawn_(false),
    pattern_index_(0),
    entry_level_(0),
    level_max_(params_.at("level_max").get<double>()),
    level_(0),
    level_speed_(vectFromJson<Vec2f>(params_.at("speed"))),
    level_yaw_(vectFromJson<Vec2f>(params_.at("yaw"))),
    level_jump_speed_(vectFromJson<Vec2f>(params_.at("jump_speed"))),
    stiff_spawn_(false),
    stiff_spawn_time_(0.0f),
    base_entry_(params_.at("base_entry").get<picojson::array>()),
    base_setup_(true),
    base_spawn_(true),
    base_spawn_index_(0)
  {
    DOUT << "Generator()" << std::endl;
  }

  ~Generator() {
    DOUT << "~Generator()" << std::endl;
  }

  
  bool isActive() const { return active_; }
  
  void message(const int msg, Signal::Params& arguments) {
    if (!active_) return;
    
    switch (msg) {
    case Msg::UPDATE:
      update(arguments);
      return;

    case Msg::MUTUAL_INTERFERENCE:
      interference(arguments);
      return;

      
    case Msg::START_GAMEMAIN:
      startGameMain();
      return;
      
    case Msg::DESTROYED_BASE:
      active_ = false;
      return;

    case Msg::ITEM_ENEMY_STIFF:
      stiff_spawn_ = true;
      stiff_spawn_time_ = boost::any_cast<float>(arguments.at("effect_time"));
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

    // 白ういろう生成準備
    if (base_setup_) setupBaseSpawn();
    
    // 黒いうろう生成準備
    if (setup_) setupEnemySpawn();
    
    // 白ういろう生成
    baseSpawn(delta_time);

    // 黒ういろう生成
    enemySpawn(delta_time);

    // アイテム効果
    if (stiff_spawn_) {
      stiff_spawn_time_ -= delta_time;
      stiff_spawn_ = (stiff_spawn_time_ >= 0.0f);
    }
  }

  
  // 白ういろう生成準備
  void setupBaseSpawn() {
    base_setup_ = false;

    if (base_spawn_index_ == base_entry_.size()) {
      // データの最後に到達していたらそこで終了
      base_spawn_ = false;
      return;
    }
    
    const auto& params = base_entry_.at(base_spawn_index_);

    base_entry_level_ = static_cast<int>(params.at("entry_level").get<double>());
    base_spawn_angle_ = deg2rad(vectFromJson<Vec2f>(params.at("angle")));

    // インデックスを進める
    base_spawn_index_ += 1;
  }
  
  // 白ういろう生成
  void baseSpawn(const float delta_time)  {
    if (!base_spawn_) return;

    // 白ういろうは黒ういろうの出現パターンの進行を待つ
    if ((base_entry_level_ >= entry_level_) || wait_clean_) return;
    
    // 生成シグナル発生
    Signal::Params params;
    params.insert(Signal::Params::value_type("spawn_pos", spawnPos(base_spawn_angle_)));

    fw_.signal().sendMessage(Msg::SPAWN_BASE, params);

    // 次の生成を指示
    base_setup_ = true;
  }
  
  // 黒ういろう生成準備
  void setupEnemySpawn() {
    DOUT << "Enemy Spawn Level:" << pattern_index_ << std::endl;
    setup_ = false;

    const auto& params = patterns_.at(pattern_index_);

    // 動作ウエイト
    wait_clean_ = params.at("wait").get<bool>();
    delay_      = params.at("delay").get<double>();

    // 生成数
    spawn_num_  = static_cast<int>(params.at("spawn_num").get<double>());
    once_spawn_ = vectFromJson<Vec2i>(params.at("once_spawn"));

    // 生成間隔
    interval_         = vectFromJson<Vec2f>(params.at("interval"));
    current_interval_ = 0.0f;

    // 生成時の位置
    angle_ = deg2rad(vectFromJson<Vec2f>(params.at("angle")));

    // 生成する種類
    // TIPS:コピーしたくないのでポインタ
    types_ = &(params.at("types").get<picojson::array>());

    // 次のパターンへ進めておく
    pattern_index_ = (pattern_index_ + 1) % patterns_.size();
    entry_level_ += 1;

    // パターンを一巡したらレベルを上げる
    if ((pattern_index_ == 0) && (level_ < level_max_)) level_ += 1;
  }
  
  // 黒ういろう生成
  void enemySpawn(const float delta_time) {
    if (!spawn_ || wait_clean_) return;
    
    if (delay_ > 0.0f) {
      delay_ -= delta_time;
      if (delay_ > 0.0f) return;
    }

    if (current_interval_ > 0.0f) {
      current_interval_ -= delta_time;
      if (current_interval_ > 0.0f) return;
    }

    // 同時生成数
    int spawn = std::min(randomValue(once_spawn_(0), once_spawn_(1)), spawn_num_);

    float level_rate = static_cast<float>(level_) / level_max_;
    
    float speed      = level_speed_(0) + (level_speed_(1) - level_speed_(0)) * level_rate;
    float yaw        = level_yaw_(0) + (level_yaw_(1) - level_yaw_(0)) * level_rate;
    float jump_speed = level_jump_speed_(0) + (level_jump_speed_(1) - level_jump_speed_(0)) * level_rate;
    
    for (int i = 0; i < spawn; ++i) {
      // 生成タイプ
      const std::string& name = types_->at(randomValue(u_int(types_->size()))).get<std::string>();

      // 生成シグナルを生成
      Signal::Params params;
      params.insert(Signal::Params::value_type("spawn_pos", spawnPos(angle_)));
      params.insert(Signal::Params::value_type("name", name));
      params.insert(Signal::Params::value_type("speed", speed));
      params.insert(Signal::Params::value_type("yaw", yaw));
      params.insert(Signal::Params::value_type("jump_speed", jump_speed));
      params.insert(Signal::Params::value_type("force_stiff", stiff_spawn_));
      params.insert(Signal::Params::value_type("force_stiff_time", stiff_spawn_time_));

      fw_.signal().sendMessage(Msg::SPAWN_ENEMY, params);
    }
    
    // 次回生成用のパラメーターを生成
    spawn_num_ -= spawn;
    if (spawn_num_ == 0) {
      setup_ = true;
    }

    current_interval_ = randomValue(interval_);
  }

  
  // 相互干渉
  void interference(const Signal::Params& arguments) {
    if (!updated_) return;

    if (wait_clean_) {
      // 黒ういろうがすべて倒されるまでは次のパターンに進まない
      wait_clean_ = Signal::isParamValue(arguments, "enemy_info");

      if (!wait_clean_) {
        // レベルアップメッセージを送信
        Signal::Params params;
        params.insert(Signal::Params::value_type("level", entry_level_));
        fw_.signal().sendMessage(Msg::GAME_LEVELUP, params);
      }
      
#ifdef _DEBUG
      if (!wait_clean_) DOUT << "Begin next level" << std::endl;
#endif
    }
  }

  // ゲーム開始
  void startGameMain() {
    // 黒ういろう生成開始
    spawn_ = true;
  }
  
  
  // [angle(0), angle(1)] の範囲で生成座標を計算
  static Vec3f spawnPos(const Vec2f& angle) {
    // [angle(0), angle(1)]の円錐内の任意のベクトルからクオータニオンを生成
    Quatf z_rot(Eigen::AngleAxisf(randomValue(angle), Vec3f::UnitZ()));
    Quatf y_rot(Eigen::AngleAxisf(randomValue(-m_pi, m_pi), Vec3f::UnitY()));

    // 生成ベクトルを算出
    return Vec3f((y_rot * z_rot) * Vec3f(0.0f, -1.0f, 0.0f));
  }
  
};

}
