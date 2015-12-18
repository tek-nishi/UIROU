
#pragma once

//
// ゲームロジック＆メニュー制御
//

#include "co_defines.hpp"
#include <sstream>
#include <iomanip>
#include <functional>
#include "nn_touchWidget.hpp"
#include "nn_textWidget.hpp"
#include "nn_menuMisc.hpp"
#include "nn_easeInOut.hpp"
#include "nn_baseHp.hpp"


namespace ngs {

class GameLogic : public ObjBase {
  Framework& fw_;
  const picojson::value& params_;
  const MatrixFont& number_font_;
  TouchWidget& touch_widget_;

  bool active_;
  bool updated_;
  bool pause_;

  bool demo_mode_;
  bool input_record_;
  
  bool in_game_;

  float play_time_;

  int destroyed_enemies_;
  int once_destroy_num_;
  int hit_combo_;
  int item_count_;
  int item_current_;

  int score_;
  int best_score_;
  
  TextWidget pause_icon_;
  TextWidget score_text_;
  TextWidget best_text_;
  TextWidget signt_text_;
  TextWidget combo_text_;
  TextWidget destroy_text_;
  TextWidget item_text_;
  TextWidget level_text_;

  std::string score_text_orig_;
  std::string best_text_orig_;
  std::string combo_text_orig_;
  std::string destroy_text_orig_;

  MiniEasing<float> mix_ease_;
  int font_mix_;

  float out_delay_;

  bool close_;
  bool game_over_;
  bool no_disp_;

  // アイテム効果
  bool  score_multiply_;
  float score_multiply_time_;

  // コンボ演出
  bool             combo_disp_;
  EaseInOut<float> combo_ease_;
  int              combo_font_mix_;
  int              combo_size_;

  // 同時破壊数演出
  bool             destroy_disp_;
  EaseInOut<float> destroy_ease_;
  int              destroy_font_mix_;
  int              destroy_size_;

  // アイテム効果演出
  bool             item_disp_;
  EaseInOut<float> item_ease_;
  int              item_font_mix_;
  int              item_size_;
  Ease<GrpCol>     item_color_ease_;
  GrpCol           item_color_;

  // レベル表示
  bool             level_disp_;
  float            level_disp_delay_;
  int              level_font_mix_;
  EaseInOut<float> level_ease_;
  std::string      level_text_orig_;
  
  // 照準
  int signt_num_;
  int signt_max_;

  // 白ういろう体力
  std::unordered_map<u_int, BaseHp> base_hp_;

  TouchWidget::Handle pause_handle_;
  bool                on_pause_;
  bool                active_pause_;

  // 光源
  Light&      light_;
  Light       light_original_;
  bool        light_effect_;
  Ease<Vec3f> light_effect_ease_;

  // BGM開始待ち
  bool  bgm_start_delay_;
  float bgm_start_delay_time_;
  
  
public:
  GameLogic(Framework& fw, const picojson::value& params,
            const MatrixFont& font, const MatrixFont& number_font, const MatrixFont& icon_font,
            TouchWidget& touch_widget, Light& light) :
    fw_(fw),
    params_(params.at("game_logic")),
    number_font_(number_font),
    touch_widget_(touch_widget),
    active_(true),
    updated_(false),
    pause_(false),
    demo_mode_(false),
    input_record_(false),
    in_game_(false),
    play_time_(0.0f),
    destroyed_enemies_(0),
    once_destroy_num_(0),
    hit_combo_(0),
    item_count_(params_.at("item_count").get<double>()),
    item_current_(0),
    score_(0),
    pause_icon_(icon_font, params_.at("pause_icon")),
    score_text_(font, params_.at("score_text")),
    best_text_(font, params_.at("best_text")),
    signt_text_(number_font, params_.at("signt_text")),
    combo_text_(font, params_.at("combo_text")),
    destroy_text_(font, params_.at("destroy_text")),
    item_text_(font, params_.at("item_text")),
    level_text_(font, params_.at("level_text")),
    score_text_orig_(score_text_.text()),
    best_text_orig_(best_text_.text()),
    combo_text_orig_(combo_text_.text()),
    destroy_text_orig_(destroy_text_.text()),
    mix_ease_(miniEasingFromJson<float>(params_.at("mix_in"))),
    out_delay_(0.0f),
    close_(false),
    game_over_(false),
    no_disp_(false),
    score_multiply_(false),
    combo_disp_(false),
    combo_ease_(params_.at("combo_in"), params_.at("combo_out"), params_.at("info_disp_duration").get<double>()),
    destroy_disp_(false),
    destroy_ease_(params_.at("destroy_in"), params_.at("destroy_out"), params_.at("info_disp_duration").get<double>()),
    item_disp_(false),
    item_ease_(params_.at("item_in"), params_.at("item_out"), 1.0f),
    item_color_ease_(easeFromJson<GrpCol>(params_.at("item_color"))),
    item_color_(1.0f, 1.0f, 1.0f, 1.0f),
    level_disp_(false),
    level_disp_delay_(false),
    level_ease_(params_.at("level_in"), params_.at("level_out"), params_.at("level_disp_duration").get<double>()),
    level_text_orig_(level_text_.text()),
    on_pause_(false),
    active_pause_(false),
    light_(light),
    light_original_(light),
    light_effect_(false),
    light_effect_ease_(easeFromJson<Vec3f>(params_.at("light_effect"))),
    bgm_start_delay_(false)
  {
    DOUT << "GameLogic()" << std::endl;

    // タッチ範囲をコールバックに登録
    pause_handle_ = menu::addWidget(touch_widget_, pause_icon_,
                                    std::bind(&GameLogic::pausehWidget,      
                                              this, std::placeholders::_1, std::placeholders::_2), false);

    // FIXME:プレイヤーの性能を直接参照している
    signt_max_ = params.at("cubePlayer").at("signt_ready_num").get<double>();
    signt_num_ = signt_max_;
    signt_text_.text(std::to_string(signt_num_));
  }

  ~GameLogic() {
    DOUT << "~GameLogic()" << std::endl;

    // ライトを元に戻しておく
    light_ = light_original_;
    
    // タッチ範囲のコールバックを解除
    touch_widget_.removeWidget(pause_handle_);
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

    case Msg::MUTUAL_INTERFERENCE:
      interference(arguments);
      return;
      
    case Msg::START_GAMEMAIN:
      startGameMain(arguments);
      return;

    case Msg::DAMAGED_BASE:
      damagedBase(arguments);
      return;
      
    case Msg::DESTROYED_BASE:
      destroyedBase(arguments);
      return;

    case Msg::DESTROYED_ENEMY:
      destroyedEnemy(arguments);
      return;

    case Msg::ATTACK_HIT_ENEMY:
      attackHitEnemy(arguments);
      return;

      
    case Msg::ITEM_MAX_POWER:
      itemMaxPower(arguments);
      return;

    case Msg::ITEM_ENEMY_STIFF:
      itemEnemyStiff(arguments);
     return;

    case Msg::ITEM_BASE_IMMORTAL:
      itemBaseImmortal(arguments);
      return;
      
    case Msg::ITEM_SCORE_MULTIPLY:
      itemScoreMultiply(arguments);
      return;

    case Msg::ITEM_MAX_RANGE:
      itemMaxRange(arguments);
      return;
      

    case Msg::SPAWN_SIGNT:
      signt_num_ = boost::any_cast<int>(arguments.at("signt_ready_num"));
      signt_text_.text(std::to_string(signt_num_));
      return;

    case Msg::RECOVER_SIGNT:
      signt_num_ = boost::any_cast<int>(arguments.at("signt_ready_num"));
      signt_text_.text(std::to_string(signt_num_));
      return;

    case Msg::GAME_LEVELUP:
      dispLevelup(arguments);
      return;

      
    case Msg::GATHER_GAME_RESULT:
      gatherGameResult(arguments);
      return;

    case Msg::FINISH_GAME_OVER:
      finishGameOver();
      return;

      
    case Msg::PAUSE_GAME:
      pause_ = true;
      return;

    case Msg::RESUME_GAME:
      pause_        = false;
      active_pause_ = true;
      touch_widget_.activeWidget(pause_handle_, true);
      return;

    case Msg::ABORT_GAME:
      abortGame();
      return;

    case Msg::FORCE_PAUSE_GAME:
      if (in_game_ && !pause_) launchPauseMenu();
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
    if (in_game_) play_time_ += delta_time;

    if (light_effect_) {
      light_.diffuse() = light_effect_ease_(delta_time);

      // ゲーム終了時にループが解除されている
      if (light_effect_ease_.isEnd()) {
        light_effect_ = false;
        light_        = light_original_;
      }
    }
    
    if (out_delay_ > 0.0f) {
      out_delay_ -= delta_time;
    }
    else if (mix_ease_.isExec()) {
      font_mix_ = static_cast<int>(mix_ease_(delta_time));
      if (!mix_ease_.isExec()) {
        if (close_) {
          // ゲーム中断
          active_ = false;
          return;
        }
        else if (game_over_) {
          // 結果画面が終わるまで非表示
          no_disp_ = true;          
        }
      }
    }

    // 結果画面表示中は何もしない
    if (no_disp_) return;

    // アイテム効果
    if (score_multiply_) {
      score_multiply_time_ -= delta_time;
      score_multiply_       = (score_multiply_time_ > 0.0f);
    }

    // コンボ表示
    if (combo_disp_) {
      combo_font_mix_ = static_cast<int>(combo_ease_(delta_time));
      combo_disp_     = combo_ease_.isExec();
    }

    // 同時破壊数表示
    if (destroy_disp_) {
      destroy_font_mix_ = static_cast<int>(destroy_ease_(delta_time));
      destroy_disp_     = destroy_ease_.isExec();
    }

    // アイテム効果表示
    if (item_disp_) {
      item_font_mix_ = static_cast<int>(item_ease_(delta_time));
      item_disp_     = item_ease_.isExec();
      item_color_    = item_color_ease_(delta_time);
    }

    // レベル表示
    if (level_disp_) {
      if (level_disp_delay_ > 0.0f) level_disp_delay_ -= delta_time;
      
      level_font_mix_ = static_cast<int>(level_ease_(delta_time));
      level_disp_     = level_ease_.isExec();
    }

    // 白ういろうの体力表示
    for (auto& hp : base_hp_) {
      hp.second.update(delta_time);
    }

    // BGM開始待ち
    if (bgm_start_delay_) {
      bgm_start_delay_time_ -= delta_time;
      if (bgm_start_delay_time_ <= 0.0f) {
        bgm_start_delay_ = false;
        gamesound::play(fw_, "bgm");
      }
    }

    
#ifdef _DEBUG
    if (fw_.keyboard().getPushed() == 'B') {
      score_ = best_score_ + 1;
      DOUT << "score overwited." << std::endl;
    }
#endif
    
  }

  void draw(Signal::Params& arguments) {
    if (!updated_ || no_disp_) return;

    // 描画プリミティブの格納先をポインタで受け取る
    auto prims = boost::any_cast<MatrixFont::PrimPack*>(arguments.at("text_prims"));

    // テキスト表示
    score_text_.draw(*prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f), font_mix_ - 13, 1);
    best_text_.draw(*prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f), font_mix_ - 10, 1);

    if (active_pause_) {
      // Pauseボタンはタップ中色を変える
      GrpCol col = on_pause_ ? GrpCol(1.0f, 0.0f, 0.0f, 1.0f) : GrpCol(1.0f, 1.0f, 1.0f, 1.0f);
      pause_icon_.draw(*prims, fw_.view(), col, font_mix_ - 2, 1);
    }
    
    signt_text_.draw(*prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f), font_mix_ - 3, 1);

    if (combo_disp_) {
      // 連続攻撃成功数
      combo_text_.draw(*prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f), combo_font_mix_ - combo_size_, 1);
    }
    if (destroy_disp_) {
      // 同時破壊数
      destroy_text_.draw(*prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f), destroy_font_mix_ - destroy_size_, 1);
    }
    if (item_disp_ &&  !pause_ && !close_) {
      item_text_.draw(*prims, fw_.view(), item_color_, item_font_mix_ - item_size_, 1);
    }
    if (level_disp_ && (level_disp_delay_ <= 0.0f) && !pause_ && !close_) {
      level_text_.draw(*prims, fw_.view(), GrpCol(0.1f, 0.1f, 0.1f, 1.0f), level_font_mix_, 1);
    }

    // 白ういろうの体力表示
    for (auto& hp : base_hp_) {
      hp.second.draw(*prims, fw_.view());
    }
  }

  
  // 生成情報を元に初期化
  void setupFromSpawnInfo(const Signal::Params& arguments) {
    best_score_ = boost::any_cast<int>(arguments.at("best_score"));

    scoreText(score_text_, score_text_orig_, score_);
    scoreText(best_text_, best_text_orig_, best_score_);

    if (Signal::isParamValue(arguments, "demo_mode")) {
      demo_mode_    = true;
      input_record_ = boost::any_cast<bool>(arguments.at("input_record"));
    }
  }

  // 相互干渉
  void interference(const Signal::Params& arguments) {
    if (!Signal::isParamValue(arguments, "base_info")) return;

    const auto& infos = boost::any_cast<const std::deque<Msg::BaseInfo>&>(arguments.at("base_info"));

    // 出現数とテキストの数が同じ場合は処理しない
    if (infos.size() ==  base_hp_.size()) return;

    float y_ofs = static_cast<float>(base_hp_.size() * params_.at("base_hp_ofs").get<double>());
    for (const auto& info : infos) {
      if (base_hp_.find(info.hash) == base_hp_.cend()) {
        Vec2f ofs(0.0f, y_ofs);
        
        // まだゲージを作ってないなら作る
        base_hp_.emplace(std::piecewise_construct,
                         std::forward_as_tuple(info.hash),
                         std::forward_as_tuple(number_font_, params_.at("base_hp_text"), ofs, info.hp_max));

        y_ofs += static_cast<float>(params_.at("base_hp_ofs").get<double>());
      }
    }
  }


  // ゲーム本編開始
  void startGameMain(const Signal::Params& arguments) {
    in_game_ = true;
    
    // 各種表示をリセット
    active_pause_ = true;
    touch_widget_.activeWidget(pause_handle_, true);

    signt_num_ = signt_max_;
    signt_text_.text(std::to_string(signt_num_));

    // BGM開始準備
    bgm_start_delay_      = true;
    bgm_start_delay_time_ = static_cast<float>(params_.get("bgm_start_delay").get<double>());

    // DEMOの場合は色々と細工する
    if (demo_mode_) {
      if (!input_record_) {
        // PAUSEメニュー無し
        active_pause_ = false;
        touch_widget_.activeWidget(pause_handle_, false);
      }
      
      // BGM ON/OFF
      bgm_start_delay_ = params_.at("demo_bgm").get<bool>();
    }
  }

  // 白ういろうダメージ
  void damagedBase(const Signal::Params& arguments) {
    u_int hash    = boost::any_cast<u_int>(arguments.at("base_hash"));
    int   hp      = boost::any_cast<int>(arguments.at("base_hp"));
    float hp_rate = boost::any_cast<float>(arguments.at("base_hp_rate"));

    // ゲージ更新
    auto& text = base_hp_.at(hash);
    text.damage(hp);

    // ピンチ演出
    if (hp_rate < 0.5f) {
      if (!light_effect_) {
        // SE
        gamesound::play(fw_, "danger");
      }
      light_effect_ = true;
    }
  }

  // 白ういろう破壊
  void destroyedBase(const Signal::Params& arguments) {
    // 得点計中断
    in_game_ = false;

    u_int hash = boost::any_cast<u_int>(arguments.at("base_hash"));
    auto& text = base_hp_.at(hash);
    text.finish();

    // メニュー入力禁止
    active_pause_ = false;
    touch_widget_.activeWidget(pause_handle_, false);
  
    // ピンチ演出終了
    light_effect_ease_.looping(false);
  }

  // 敵に攻撃が当たった
  void attackHitEnemy(const Signal::Params& arguments) {
    if (!in_game_) return;

    // 連続攻撃成功数
    int combo_num = boost::any_cast<int>(arguments.at("hit_combo"));
    hit_combo_ = std::max(combo_num, hit_combo_);
    comboText(combo_num);
  }
  
  // 敵破壊時の処理
  void destroyedEnemy(const Signal::Params& arguments) {
    if (!in_game_) return;
    
    // 撃破数集計
    int destroy_num = boost::any_cast<int>(arguments.at("destroy"));
    destroyed_enemies_ += destroy_num;
    once_destroy_num_ = std::max(destroy_num, once_destroy_num_);
    // テキスト表示
    destroyText(destroy_num);

    // 一定撃破数ごとにアイテム出現
    int item_count = destroyed_enemies_ / item_count_;
    if (item_count > item_current_) {
      u_int num = item_count - item_current_;
      const auto& info = boost::any_cast<const std::deque<Msg::EnemyInfo>&>(arguments.at("destroy_enemy"));
      assert(num <= info.size());
      for (u_int i = 0; i < num; ++i) {
        Signal::Params params;
        info[i].obj->message(Msg::ENEMY_SPAWN_ITEM, params);
      }

      item_current_ = item_count;
    }

    // 連続攻撃成功数
    int combo_num = boost::any_cast<int>(arguments.at("hit_combo"));
    hit_combo_ = std::max(combo_num, hit_combo_);
    // テキスト表示
    comboText(combo_num);

    // スコア計算(同時破壊数とHIT-COMBOが続くほど高得点)
    int score = 10 * (destroy_num * destroy_num + (combo_num - 1) * 0.25f);
    // アイテム効果
    if (score_multiply_) score *= 4;

    score_ += score;

    // スコア表示のテキストを作り直す
    scoreText(score_text_, score_text_orig_, score_);

    // Best更新
    if (best_score_ < score_) {
      best_score_ = score_;
      scoreText(best_text_, best_text_orig_, best_score_);
    }

    // 実績
    if (destroy_num >= 2) {
      gamecenter::achievement("NGS0004UIROU.combo2", 100.0, demo_mode_);
    }
    if (destroy_num >= 5) {
      gamecenter::achievement("NGS0004UIROU.combo5", 100.0, demo_mode_);
    }

    if (combo_num == 10) {
      gamecenter::achievement("NGS0004UIROU.hitcombo10", 100.0, demo_mode_);
    }
    if (combo_num == 50) {
      gamecenter::achievement("NGS0004UIROU.hitcombo50", 100.0, demo_mode_);
    }
    if (combo_num == 100) {
      gamecenter::achievement("NGS0004UIROU.hitcombo100", 100.0, demo_mode_);
    }
  }

  
  // アイテム効果: 攻撃力最大
  void itemMaxPower(const Signal::Params& arguments) {
    itemEffectText(arguments, "max_power");
  }

  // アイテム効果: 敵硬直
  void itemEnemyStiff(const Signal::Params& arguments) {
    itemEffectText(arguments, "enemy_stiff");
  }

  // アイテム効果: 白ういろう無敵
  void itemBaseImmortal(const Signal::Params& arguments) {
    itemEffectText(arguments, "base_immortal");
  }

  // アイテム効果: スコアX4
  void itemScoreMultiply(const Signal::Params& arguments) {
    score_multiply_      = true;
    score_multiply_time_ = boost::any_cast<float>(arguments.at("effect_time"));

    itemEffectText(arguments, "score_multiply");
  }

  // アイテム効果: 攻撃範囲最大
  void itemMaxRange(const Signal::Params& arguments) {
    itemEffectText(arguments, "max_range");
  }
  
  
  // プレイ結果の収拾
  void gatherGameResult(Signal::Params& arguments) {
    arguments.insert(Signal::Params::value_type("play_time", play_time_));
    arguments.insert(Signal::Params::value_type("destroyed_enemies", destroyed_enemies_));

    arguments.insert(Signal::Params::value_type("score", score_));
    arguments.insert(Signal::Params::value_type("once_destroy_num", once_destroy_num_));
    arguments.insert(Signal::Params::value_type("hit_combo_num", hit_combo_));
  }


  // ゲーム中断時のテキスト消去開始
  void abortGame() {
    pause_     = false;
    close_     = true;
    out_delay_ = params_.at("out_delay").get<double>();
    mix_ease_  = miniEasingFromJson<float>(params_.at("mix_out"));

    light_effect_ = false;
    
    // テキスト表示の後始末
    combo_ease_.doEnd();
    destroy_ease_.doEnd();
    item_ease_.stop();
    level_ease_.stop();

    // 白ういろうの体力表示の後始末
    for (auto& hp : base_hp_) {
      hp.second.finish();
    }

    // BGM再生予約解除
    bgm_start_delay_ = false;
  }

  // ゲーム終了後テキスト消去開始
  void finishGameOver() {
    game_over_ = true;
    out_delay_ = params_.at("out_delay").get<double>();
    mix_ease_  = miniEasingFromJson<float>(params_.at("mix_out"));

    // テキスト表示の後始末
    combo_ease_.doEnd();
    destroy_ease_.doEnd();
    item_ease_.doEnd();

    // 白ういろうの体力表示の後始末
    for (auto& hp : base_hp_) {
      hp.second.finish();
    }
  }

  
  // スコアテキスト生成
  void scoreText(TextWidget& widget, const std::string& pre_text, const int score) {
    std::ostringstream text;
    text << pre_text << std::setw(6) << std::setfill('0') << score;
    widget.text(text.str());
  }

  // 同時破壊数テキスト表示
  void destroyText(const int destroy_num) {
    if (destroy_num > 1) {
      destroy_disp_ = true;
      
      destroy_text_.text(std::to_string(destroy_num) + destroy_text_orig_);
      destroy_size_ = int(destroy_text_.text().size()) + 1;

      destroy_ease_.inStart(destroy_size_ + 2.0f);
      destroy_ease_.outEnd(destroy_size_ + 2.0f);
      destroy_ease_.start();
    }
  }
  
  // 連続攻撃成功テキスト生成
  void comboText(const int combo_num) {
    if (combo_num > 1) {
      combo_disp_ = true;

      combo_text_.text(std::to_string(combo_num) + combo_text_orig_);
      combo_size_ = int(combo_text_.text().size()) + 1;

      combo_ease_.inStart(combo_size_ + 2.0f);
      combo_ease_.outEnd(combo_size_ + 2.0f);
      combo_ease_.start();
    }
  }

  // アイテム効果テキスト生成
  void itemEffectText(const Signal::Params& arguments, const std::string& param_name) {
    item_disp_ = true;

    item_text_.text(params_.at("item_effect").at(param_name).get<std::string>());
    item_size_ = int(item_text_.text().size()) + 1;

    item_ease_.inStart(item_size_ + 2.0f);
    item_ease_.outEnd(item_size_ + 2.0f);
    item_ease_.duration(boost::any_cast<float>(arguments.at("effect_time")) - 1.0f);
    item_ease_.start();
  }

  // レベルアップ演出開始
  void dispLevelup(const Signal::Params& arguments) {
    int level = boost::any_cast<int>(arguments.at("level"));

    if (level == 1) return;
    
    std::string text = level_text_orig_ + " " + std::to_string(level);
    level_text_.text(text);
    
    level_disp_       = true;
    level_disp_delay_ = static_cast<float>(params_.at("level_disp_delay").get<double>());
    level_ease_.start();
    level_font_mix_ = level_ease_(0.0f);
  }
  

  // ポーズ制御
  void pausehWidget(const TouchWidget::Msg msg, const Vec2f& pos) {
    switch (msg) {
    case TouchWidget::TOUCH_START:
      {
        DOUT << "TOUCH_START" << std::endl;
        on_pause_ = true;

        // 照準操作は中断
        Signal::Params params;
        fw_.signal().sendMessage(Msg::MANIPULATE_ONCE_SKIP, params);
      }
      break;
      
    case TouchWidget::MOVE_OUT_IN:
      DOUT << "MOVE_OUT_IN" << std::endl;
      on_pause_ = true;
      break;
      
    // case TouchWidget::MOVE_IN:
    //   DOUT << "MOVE_IN" << std::endl;
    //   break;
      
    case TouchWidget::MOVE_IN_OUT:
      DOUT << "MOVE_IN_OUT" << std::endl;
      on_pause_ = false;
      break;
      
    // case TouchWidget::MOVE_OUT:
    //   DOUT << "MOVE_OUT" << std::endl;
    //   break;
      
    case TouchWidget::TOUCH_END_IN:
      DOUT << "TOUCH_END_IN" << std::endl;
      // ポーズメニュー起動
      launchPauseMenu();
      break;
      
    case TouchWidget::TOUCH_END_OUT:
      DOUT << "TOUCH_END_OUT" << std::endl;
      on_pause_ = false;
      break;

    default:
      break;
    }
  }

  // ポーズメニュー起動
  void launchPauseMenu() {
    on_pause_     = false;
    active_pause_ = false;
    touch_widget_.activeWidget(pause_handle_, false);

    // ポーズメニュー起動
    Signal::Params param;
    fw_.signal().sendMessage(Msg::PAUSE_GAME, param);
  }
  
};

}
