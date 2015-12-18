
#pragma once

//
// 結果画面
//

#include "co_easing.hpp"
#include "nn_menuMisc.hpp"
#include "nn_gameSound.hpp"
#include "sns.h"
#include "gamecenter.h"
#include "rating.h"


namespace ngs {

class GameResult : public ObjBase {
  Framework& fw_;
  const picojson::value& params_;
  const picojson::value& uirou_params_;
  TouchWidget& touch_widget_;
  const Camera& camera_;

  bool active_;
  bool updated_;

  // 表示テキスト
  TextWidget title_;
  TextWidget sub_score_;
  TextWidget sub_new_best_;
  std::deque<TextWidget> text_;

  TextWidget icon_twitter_;
  TextWidget icon_facebook_;
  
  // プレイ結果
  float play_time_;
  int   destroyed_enemies_;
  int   once_destroy_num_;
  int   combo_hit_num_;
  int   score_;
  int   best_score_;
  bool  new_best_;
  int   rank_;

  MiniEasing<float> mix_ease_;
  int font_mix_;

  float touch_exec_delay_;

  bool close_;
  
  bool twitter_;
  bool facebook_;
  TouchWidget::Handle twitter_handle_;
  TouchWidget::Handle facebook_handle_;
  bool on_twitter_;
  bool on_facebook_;

  std::string sns_text_;

#if defined (USE_GAMECENTER)
  TextWidget          gamecenter_;
  TextWidget          gamecenter_text_;
  TouchWidget::Handle gamecenter_handle_;
  bool                on_gamecenter_;
  bool                all_ranks_;
#endif
  
  GrpCol score_color_;

  bool best_effect_;
  Ease<GrpCol> best_color_;

  MiniEasing<float> best_ease_;
  float best_ease_delay_;

  Model model_;
  Vec3f model_color_;

  std::shared_ptr<EasyShader> shader_;

  Eigen::Affine3f model_matrix_;

  Quatf spin_rotate_;
  float spin_speed_;
  float spin_angle_;

  Vec3f  cube_pos_;
  View::Layout cube_layout_;
  float  cube_size_;

  float cube_scale_;
  float scale_delay_;
  MiniEasing<float> scale_ease_;
  Ease<float> color_ease_;
  
  
public:
  GameResult(Framework& fw, const picojson::value& params,
             ModelHolder& model_holder, ShaderHolder& shader_holder,
             const MatrixFont& font, const MatrixFont& icon_font,
             TouchWidget& touch_widget,
             const Camera& camera) :
    fw_(fw),
    params_(params.at("game_result")),
    uirou_params_(params.at("uirous")),
    touch_widget_(touch_widget),
    camera_(camera),
    active_(true),
    updated_(false),
    title_(font, params_.at("title")),
    sub_score_(font, params_.at("sub_score")),
    sub_new_best_(font, params_.at("sub_new_best")),
    icon_twitter_(icon_font, params_.at("twitter")),
    icon_facebook_(icon_font, params_.at("facebook")),
    mix_ease_(miniEasingFromJson<float>(params_.at("mix_in"))),
    touch_exec_delay_(0.0f),
    close_(false),
    twitter_(false),
    facebook_(false),
    on_twitter_(false),
    on_facebook_(false),
#if defined (USE_GAMECENTER)
    gamecenter_(icon_font, params_.at("gamecenter")),
    gamecenter_text_(font, params_.at("gamecenter_text")),
    on_gamecenter_(false),
    all_ranks_(false),
#endif
    score_color_(1.0f, 1.0f, 1.0f, 1.0f),
    best_effect_(false),
    best_color_(easeFromJson<GrpCol>(params_.at("best_color"))),
    best_ease_(miniEasingFromJson<float>(params_.at("best_ease"))),
    best_ease_delay_(0.0f),
    model_(model_holder.read(uirou_params_.at("model").get<std::string>())),
    shader_(shader_holder.read(uirou_params_.at("shader").get<std::string>())),
    spin_rotate_(quatFromJson(params_.at("spin_rotate"))),
    spin_speed_(deg2rad(static_cast<float>(params_.at("spin_speed").get<double>()))),
    spin_angle_(0.0f),
    cube_pos_(vectFromJson<Vec3f>(params_.at("cube_pos"))),
    cube_layout_(View::layoutFromString(params_.at("cube_layout").get<std::string>())),
    cube_size_(params_.at("cube_size").get<double>()),
    cube_scale_(0.0f),
    scale_delay_(params_.at("scale_delay").get<double>()),
    scale_ease_(miniEasingFromJson<float>(uirou_params_.at("scale_disp"))),
    color_ease_(easeFromJson<float>(uirou_params_.at("color_disp")))
  {
    DOUT << "GameResult()" << std::endl;

    // 表示するテキストをコンテナに積む
    const picojson::array& text = params_.at("text").get<picojson::array>();
    for (const auto& value : text) {
      text_.emplace_back(font, value);
    }
    
    // Best更新演出は、読み込んだら一時停止にしておく
    best_ease_.stop();

    // タッチ範囲をコールバックに登録
    twitter_handle_ = menu::addWidget(touch_widget_, icon_twitter_,
                                      std::bind(&GameResult::twitterWidget,        
                                                this, std::placeholders::_1, std::placeholders::_2), false);

    facebook_handle_ = menu::addWidget(touch_widget_, icon_facebook_,
                                       std::bind(&GameResult::facebookWidget,         
                                                 this, std::placeholders::_1, std::placeholders::_2), false);
#if defined (USE_GAMECENTER)
    gamecenter_handle_ = menu::addWidget(touch_widget_, gamecenter_,
                                         std::bind(&GameResult::gamecenterWidget,
                                                   this, std::placeholders::_1, std::placeholders::_2), false);
#endif

    // SE
    gamesound::play(fw_, "ranking");
  }

  ~GameResult() {
    DOUT << "~GameResult()" << std::endl;

    touch_widget_.removeWidget(twitter_handle_);
    touch_widget_.removeWidget(facebook_handle_);
#if defined (USE_GAMECENTER)
    touch_widget_.removeWidget(gamecenter_handle_);
#endif
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
      
    case Msg::FINISH_SKIP_TAP:
      skipTap();
      return;
      
    case Msg::FINISH_AWAIT_TAP:
      finishGame();
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
    updated_ = true;

    float delta_time = boost::any_cast<float>(arguments.at("delta_time"));
    
    // 結果表示演出
    if (mix_ease_.isExec()) {
      font_mix_ = static_cast<int>(mix_ease_(delta_time));
      if (!mix_ease_.isExec()) {
        if (close_) {
          // ゲーム終了
          active_ = false;

          rating::popup();
          
          Signal::Params params;
          fw_.signal().sendMessage(Msg::END_GAME, params);
          return;
        }
        else {
          touch_exec_delay_ = params_.at("touch_exec_delay").get<double>();
        }
      }
    }

    // 「HIGH SCORE!」表示待ち
    if (best_ease_delay_ > 0.0f) {
      best_ease_delay_ -= delta_time;
      if (best_ease_delay_ <= 0.0f) {
        // 演出開始
        best_ease_.resume();
      }
    }
    
    // 「HIGH SCORE!」表示演出
    if (best_ease_.isExec()) {
      sub_new_best_.scale(best_ease_(delta_time));
      // スコア色変化演出開始
      if (!best_ease_.isExec()) {
        // 振動演出
        Signal::Params params;
        // SE
        gamesound::play(fw_, "high_score");

        params.insert(Signal::Params::value_type("power", 5.0f));
        params.insert(Signal::Params::value_type("speed", 22.0f));
      
        fw_.signal().sendMessage(Msg::QUAKE_CAMERA, params);
        
        best_effect_ = true;
      }
    }
    
    // スコア色変化演出
    if (best_effect_) score_color_ = best_color_(delta_time);
    
    // 「次の画面へ」表示待ち
    if (touch_exec_delay_ > 0.0f) {
      touch_exec_delay_ -= delta_time;
      if (touch_exec_delay_ <= 0.0f) {
        // スキップ操作停止
        Signal::Params param;
        fw_.signal().sendMessage(Msg::ABORT_SKIP_TAP, param);

        // 「次の画面へ」準備
        setupAwaitTap();
      }
    }
    
    spin_angle_ += spin_speed_ * delta_time;

    float color = color_ease_(delta_time);
    Model::materialEmissiveColor(model_, model_color_ * color);

    if (scale_delay_ > 0.0f) {
      scale_delay_ -= delta_time;
      if (scale_delay_ <= 0.0f) {
        // SE
        gamesound::play(fw_, "rank");
      }
    }
    else {
      cube_scale_ = scale_ease_(delta_time);
    }

    // ランクのういろうはメニューと同じ2D座標で指定されているので、それを3D座標に変換する
    Vec3f cube_pos = menu::layoutPosToWorld(cube_pos_, cube_layout_, fw_.view(), camera_.projection(), Mat4f::Identity());
    
    model_matrix_ =
      Eigen::Translation<float, 3>(cube_pos)
      * Eigen::AngleAxisf(spin_angle_, Vec3f::UnitY())
      * spin_rotate_
      * Eigen::Scaling(Vec3f(cube_size_ * cube_scale_, cube_size_ * cube_scale_, cube_size_ * cube_scale_));
  }

  // 描画
  void draw(Signal::Params& arguments) {
    if (!updated_) return;

    pushMatrix();
    loadMatrix(Mat4f::Identity());

    modelDraw(model_, model_matrix_.matrix(), *shader_, true);
      
    popMatrix();
    
    // 描画プリミティブの格納先をポインタで受け取る
    auto prims = boost::any_cast<MatrixFont::PrimPack*>(arguments.at("text_prims"));
    
    // 結果表示
    title_.draw(*prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f), font_mix_ - 40, 1);
    for (auto& t : text_) {
      t.draw(*prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f), font_mix_ - 25, 1);
    }

    // スコア表示
    sub_score_.draw(*prims, fw_.view(), score_color_, font_mix_ - 20, 1);
    if (new_best_ && best_ease_delay_ <= 0.0f) {
      sub_new_best_.draw(*prims, fw_.view(), GrpCol(1.0f, 0.0f, 0.0f, 1.0f), font_mix_ - 15, 1);
    }

    // アイコン表示
    if (twitter_) {
      // アイコンは状況で表示色を変更する
      GrpCol col = on_twitter_ ? GrpCol(1.0f, 0.0f, 0.0f, 1.0f) : GrpCol(1.0f, 1.0f, 1.0f, 1.0f);
      icon_twitter_.draw(*prims, fw_.view(), col, font_mix_ - 10, 1);
    }
    if (facebook_) {
      // アイコンは状況で表示色を変更する
      GrpCol col = on_facebook_ ? GrpCol(1.0f, 0.0f, 0.0f, 1.0f) : GrpCol(1.0f, 1.0f, 1.0f, 1.0f);
      icon_facebook_.draw(*prims, fw_.view(), col, font_mix_ - 9, 1);
    }
    
#if defined (USE_GAMECENTER)
    // アイコンは状況で表示色を変更する
    GrpCol col = on_gamecenter_ ? GrpCol(1.0f, 0.0f, 0.0f, 1.0f) : GrpCol(1.0f, 1.0f, 1.0f, 1.0f);
    gamecenter_.draw(*prims, fw_.view(), col, font_mix_ - 8, 1);
    gamecenter_text_.draw(*prims, fw_.view(), col, font_mix_ - 12, 1);
#endif
  }

  
  // 初期設定
  void setupFromSpawnInfo(const Signal::Params& arguments) {
    play_time_         = boost::any_cast<float>(arguments.at("play_time"));
    destroyed_enemies_ = boost::any_cast<int>(arguments.at("destroyed_enemies"));
    once_destroy_num_  = boost::any_cast<int>(arguments.at("once_destroy_num"));
    combo_hit_num_     = boost::any_cast<int>(arguments.at("hit_combo_num"));
    score_             = boost::any_cast<int>(arguments.at("score"));
    best_score_        = boost::any_cast<int>(arguments.at("best_score"));
    new_best_          = boost::any_cast<bool>(arguments.at("new_best"));
    rank_              = boost::any_cast<int>(arguments.at("rank"));

    twitter_  = boost::any_cast<bool>(arguments.at("twitter"));
    facebook_ = boost::any_cast<bool>(arguments.at("facebook"));

#if defined (USE_GAMECENTER)
    all_ranks_ = boost::any_cast<bool>(arguments.at("all_ranks"));
#endif

    // 各種テキスト書き換え
    sub_score_.text(sub_score_.text() + std::to_string(score_));

    std::string play_time = textFromTime(play_time_);
    std::string destroyed_enemies = std::to_string(destroyed_enemies_);
    std::string once_destroy_num = std::to_string(once_destroy_num_);
    std::string combo_hit_num = std::to_string(combo_hit_num_);
    std::string best_score = std::to_string(best_score_);
    const std::string& rank_text = getRankText(rank_);

    // %1 プレイ時間
    // %2 敵破壊数
    // %3 一度に破壊した数
    // %4 ベストスコア
    // %5 ランク
    // %6 連続HIT
    for (auto& t : text_) {
      std::string text = t.text();
      replaceString(text, "%1", play_time);
      replaceString(text, "%2", destroyed_enemies);
      replaceString(text, "%3", once_destroy_num);
      replaceString(text, "%4", best_score);
      replaceString(text, "%5", rank_text);
      replaceString(text, "%6", combo_hit_num);
      t.text(text);
    }

    // 表示モデルの色はランクで決まる
    model_color_ = vectFromJson<Vec3f>(uirou_params_.at("model_colors").at(rank_text));
    // テクスチャ差し替え
    model_.materialTexture(fw_.loadPath() + uirou_params_.at("model_textures").at(rank_text).get<std::string>());

    if (new_best_) {
      best_ease_delay_ = params_.at("new_best_delay").get<double>();
    }
  }
  
  // 演出スキップ操作
  void skipTap() {
    if (!updated_) return;

    // フォント演出停止
    mix_ease_.stop();
    font_mix_ = 0;

    // BEST演出停止
    best_ease_delay_ = 0.0f;
    best_ease_.stop();
    if (new_best_) {
      sub_new_best_.scale(best_ease_.endValue());
      best_effect_ = true;
    }

    // Rank表示演出停止
    scale_delay_ = 0.0f;
    scale_ease_.toEnd();

    touch_exec_delay_ = 0.0f;

    // 「次の画面へ」準備
    setupAwaitTap();
  }

  // ゲーム終了
  void finishGame() {
    // 記録画面終了演出開始
    close_      = true;
    mix_ease_   = miniEasingFromJson<float>(params_.at("mix_out"));
    scale_ease_ = miniEasingFromJson<float>(uirou_params_.at("scale_erase"));

    gamesound::play(fw_, "wipe");
  }

  // 「次の画面へ」準備
  void setupAwaitTap() {
    if (twitter_)  touch_widget_.activeWidget(twitter_handle_, true);
    if (facebook_) touch_widget_.activeWidget(facebook_handle_, true);
#if defined (USE_GAMECENTER)
    touch_widget_.activeWidget(gamecenter_handle_, true);

    if (all_ranks_) {
      gamecenter::achievement("NGS0004UIROU.allrank", 100.0);
    }
#endif
    
    Signal::Params param;
    fw_.signal().sendMessage(Msg::START_AWAIT_TAP, param);
  }
  

  // twitter投稿
  void twitterWidget(const TouchWidget::Msg msg, const Vec2f& pos) {
    switch (msg) {
    case TouchWidget::TOUCH_START:
      on_twitter_ = true;
      break;
      
    case TouchWidget::MOVE_OUT_IN:
      on_twitter_ = true;
      break;
      
    // case TouchWidget::MOVE_IN:
    //   break;
      
    case TouchWidget::MOVE_IN_OUT:
      on_twitter_ = false;
      break;
      
    // case TouchWidget::MOVE_OUT:
    //   break;
      
    case TouchWidget::TOUCH_END_IN:
      {
        // 投稿する
        on_twitter_ = false;

        std::string text = sns::postText(sns::TWITTER);
        arrangeSnsText(text);
        sns::post(sns::TWITTER, text);

        // 照準操作は中断
        Signal::Params params;
        fw_.signal().sendMessage(Msg::MANIPULATE_ONCE_SKIP, params);
      }
      break;
      
    case TouchWidget::TOUCH_END_OUT:
      on_twitter_ = false;
      break;

    default:
      break;
    }
  }

  // facebook投稿
  void facebookWidget(const TouchWidget::Msg msg, const Vec2f& pos) {
    switch (msg) {
    case TouchWidget::TOUCH_START:
      on_facebook_ = true;
      break;
      
    case TouchWidget::MOVE_OUT_IN:
      on_facebook_ = true;
      break;
      
    // case TouchWidget::MOVE_IN:
    //   break;
      
    case TouchWidget::MOVE_IN_OUT:
      on_facebook_ = false;
      break;
      
    // case TouchWidget::MOVE_OUT:
    //   break;
      
    case TouchWidget::TOUCH_END_IN:
      {
        // 投稿する
        on_facebook_ = false;

        std::string text = sns::postText(sns::FACEBOOK);
        arrangeSnsText(text);
        sns::post(sns::FACEBOOK, text);

        // 照準操作は中断
        Signal::Params params;
        fw_.signal().sendMessage(Msg::MANIPULATE_ONCE_SKIP, params);
      }
      break;
      
    case TouchWidget::TOUCH_END_OUT:
      on_facebook_ = false;
      break;

    default:
      break;
    }
  }
  
#if defined (USE_GAMECENTER)
  // GameCenter
  void gamecenterWidget(const TouchWidget::Msg msg, const Vec2f& pos) {
    switch (msg) {
    case TouchWidget::TOUCH_START:
      {
        on_gamecenter_ = true;

        // 照準操作は中断
        Signal::Params params;
        fw_.signal().sendMessage(Msg::MANIPULATE_ONCE_SKIP, params);
      }
      break;
      
    case TouchWidget::MOVE_OUT_IN:
      on_gamecenter_ = true;
      break;
      
    // case TouchWidget::MOVE_IN:
    //   break;
      
    case TouchWidget::MOVE_IN_OUT:
      on_gamecenter_ = false;
      break;
      
    // case TouchWidget::MOVE_OUT:
    //   break;
      
    case TouchWidget::TOUCH_END_IN:
      {
        // GameCenter画面表示
        on_gamecenter_ = false;
        gamecenter::showGamecenter();
        gamesound::play(fw_, "wipe");
      }
      break;
      
    case TouchWidget::TOUCH_END_OUT:
      on_gamecenter_ = false;
      break;

    default:
      break;
    }
  }
#endif


  // ランクのテキストを取得
  const std::string& getRankText(int rank) {
    return uirou_params_.at("rank").get(rank).get<std::string>();
  }

  // SNS投稿テキストを整形する
  void arrangeSnsText(std::string& text) {
    replaceString(text, "%1", std::to_string(score_));
    replaceString(text, "%2", getRankText(rank_));
  }
  
};

}
