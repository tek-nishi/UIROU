
#pragma once

//
// 総合記録
//

#include "nn_menuMisc.hpp"
#include "nn_gameSound.hpp"
#include "gamecenter.h"


namespace ngs {

class Records : public ObjBase {
  Framework& fw_;
  const picojson::value& params_;
  const picojson::value& uirou_params_;
  const picojson::value& settings_;
  const Camera& camera_;
  TouchWidget& touch_widget_;

  bool active_;
  bool updated_;

  // 表示テキスト
  TextWidget title_;
  std::deque<TextWidget> text_;

#if defined (USE_GAMECENTER)
  TextWidget          gamecenter_;
  TextWidget          gamecenter_text_;
  TouchWidget::Handle gamecenter_handle_;
  bool                on_gamecenter_;
#endif

  enum Mode {
    EFFECT_IN,
    TAP_DELAY,
    TAP_WAIT,
    EFFECT_OUT
  };
  Mode mode_;
  
  MiniEasing<float> mix_ease_;
  int font_mix_;

  float touch_exec_delay_;

  struct Uirou {
    Model model;
    Vec3f color;
    Eigen::Affine3f matrix;
    MiniEasing<float> scale_ease;
    float delay;
    float scale;
    bool ease_in;
    TextWidget text;

    Uirou(ModelHolder& model_holder,
          const std::string& model_name,
          const Vec3f& model_color,
          const std::string& model_texture,
          const MiniEasing<float>& model_scale_ease,
          const float scale_delay,
          const MatrixFont& font,
          const picojson::value& json,
          const std::string& rank_name) :
      model(model_holder.read(model_name)),
      color(model_color),
      scale_ease(model_scale_ease),
      delay(scale_delay),
      scale(0.0f),
      ease_in(true),
      text(font, json)
    {
      // マテリアルのテクスチャ変更
      model.materialTexture(model_texture);
      // 表示テキスト
      text.text(rank_name);
    }
  };

  std::deque<Uirou> rank_uirous_;

  std::shared_ptr<EasyShader> shader_;

  Quatf spin_rotate_;
  float spin_speed_;
  float spin_angle_;
  Vec3f cube_pos_;
  View::Layout cube_layout_;
  float cube_size_;
  Vec2f cube_gap_;
  float rank_text_ofs_;
  
  Ease<float> color_ease_;
  
  
public:
  Records(Framework& fw, const picojson::value& params,
          ModelHolder& model_holder, ShaderHolder& shader_holder,
          const picojson::value& settings,
          const MatrixFont& font, const MatrixFont& icon_font,
          const Camera& camera, TouchWidget& touch_widget) :
    fw_(fw),
    params_(params.at("records")),
    uirou_params_(params.at("uirous")),
    settings_(settings),
    camera_(camera),
    touch_widget_(touch_widget),
    active_(true),
    updated_(false),
    title_(font, params_.at("title")),
#if defined (USE_GAMECENTER)
    gamecenter_(icon_font, params_.at("gamecenter")),
    gamecenter_text_(font, params_.at("gamecenter_text")),
    on_gamecenter_(false),
#endif
    mode_(EFFECT_IN),
    mix_ease_(miniEasingFromJson<float>(params_.at("mix_in"))),
    shader_(shader_holder.read(uirou_params_.at("shader").get<std::string>())),
    spin_rotate_(quatFromJson(params_.at("spin_rotate"))),
    spin_speed_(deg2rad(static_cast<float>(params_.at("spin_speed").get<double>()))),
    spin_angle_(0.0f),
    cube_pos_(vectFromJson<Vec3f>(params_.at("cube_pos"))),
    cube_layout_(View::layoutFromString(params_.at("cube_layout").get<std::string>())),
    cube_size_(params_.at("cube_size").get<double>()),
    cube_gap_(vectFromJson<Vec2f>(params_.at("cube_gap"))),
    rank_text_ofs_(params_.at("rank_text_ofs").get<double>()),
    color_ease_(easeFromJson<float>(uirou_params_.at("color_disp")))
  {
    DOUT << "Records()" << std::endl;

    // 表示するテキストをコンテナに積む
    const picojson::array& text_array = params_.at("text").get<picojson::array>();
    for (const auto& value : text_array) {
      text_.emplace_back(font, value);
    }

    // テキスト置換
    //  %1 総プレイ回数
    //  %2 総プレイ時間
    //  %3 総破壊数
    //  %4 最大同時破壊
    //  %5 最大HITコンボ
    std::string number_of_played  = std::to_string(int(settings_.at("number_of_played").get<double>()));
    std::string time_played       = textFromTime(settings_.at("time_played").get<double>());
    std::string total_destroyed   = std::to_string(int(settings_.at("total_destroyed").get<double>()));
    std::string most_once_destroy = std::to_string(int(settings_.at("most_once_destroy").get<double>()));
    std::string most_combo_hit    = std::to_string(int(settings_.at("most_combo_hit").get<double>()));
    
    for (auto& t : text_) {
      std::string text = t.text();
      replaceString(text, "%1", number_of_played);
      replaceString(text, "%2", time_played);
      replaceString(text, "%3", total_destroyed);
      replaceString(text, "%4", most_once_destroy);
      replaceString(text, "%5", most_combo_hit);
      t.text(text);
    }

    // 取得したランクを表示
    const auto& all_ranks = settings_.at("all_ranks");

    // マテリアル色
    const auto& model_colors = uirou_params_.at("model_colors");
    // テクスチャ
    const auto& model_textures = uirou_params_.at("model_textures");

    // ランクの並び順に調べる
    const auto& rank_order = uirou_params_.at("rank").get<picojson::array>();
    float delay = params_.at("delay").get<double>();
    float delay_interval = params_.at("delay_interval").get<double>();
    for (auto it = rank_order.crbegin(); it != rank_order.crend(); ++it) {
      const std::string& rank_name = it->get<std::string>();
      if (all_ranks.get(rank_name).is<bool>()) {
        // 取得uirouの表示設定をコンテナに積む
        rank_uirous_.emplace_back(model_holder, uirou_params_.at("model").get<std::string>(),
                                  vectFromJson<Vec3f>(model_colors.at(rank_name)),
                                  fw_.loadPath() + model_textures.at(rank_name).get<std::string>(),
                                  miniEasingFromJson<float>(uirou_params_.at("scale_disp")),
                                  delay,
                                  font, params_.at("rank_text"), rank_name);

        delay += delay_interval;
      }
    }
    
#if defined (USE_GAMECENTER)
    gamecenter_handle_ = menu::addWidget(touch_widget_, gamecenter_,
                                         std::bind(&Records::gamecenterWidget,
                                                   this, std::placeholders::_1, std::placeholders::_2), false);
#endif
  }

  ~Records() {
    DOUT << "~Records()" << std::endl;
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


    case Msg::FINISH_SKIP_TAP:
      if (updated_) {
        // 演出スキップ操作
        switch (mode_) {
        case EFFECT_IN:
        case TAP_DELAY:
          mode_ = TAP_DELAY;
          font_mix_ = 0;
          touch_exec_delay_ = 0.0f;

          for (auto& uirou : rank_uirous_) {
            uirou.delay   = 0.0f;
            uirou.ease_in = false;
            uirou.scale_ease.toEnd();
          }

#if defined (USE_GAMECENTER)
          touch_widget_.activeWidget(gamecenter_handle_, true);
#endif
          break;

        default:
          break;
        }
      }
      return;
      
    case Msg::FINISH_AWAIT_TAP:
      {
        // 終了演出開始
        mode_ = EFFECT_OUT;
        mix_ease_ = miniEasingFromJson<float>(params_.at("mix_out"));

        gamesound::play(fw_, "wipe");

        float delay = 0.0f;
        float delay_interval = params_.at("erase_interval").get<double>();
        for (auto& uirou : rank_uirous_) {
          uirou.delay = delay;
          uirou.scale_ease = miniEasingFromJson<float>(uirou_params_.at("scale_erase"));
                                                       
          delay += delay_interval;
        }
      }
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

    switch (mode_) {
    case EFFECT_IN:
      // 画面表示演出
      font_mix_ = static_cast<int>(mix_ease_(delta_time));
      if (!mix_ease_.isExec()) {
        touch_exec_delay_ = params_.at("touch_exec_delay").get<double>();
        mode_ = TAP_DELAY;
      }
      break;

    case TAP_DELAY:
      // 「次の画面へ」表示待ち
      touch_exec_delay_ -= delta_time;
      if (touch_exec_delay_ <= 0.0f) {
        Signal::Params param;
        fw_.signal().sendMessage(Msg::ABORT_SKIP_TAP, param);
        fw_.signal().sendMessage(Msg::START_AWAIT_TAP, param);
        mode_ = TAP_WAIT;

#if defined (USE_GAMECENTER)
        touch_widget_.activeWidget(gamecenter_handle_, true);
#endif
      }
      break;

    case TAP_WAIT:
      // 入力待ち
      break;

    case EFFECT_OUT:
      font_mix_ = static_cast<int>(mix_ease_(delta_time));
      if (!mix_ease_.isExec()) {
        // 記録画面を終了してタイトルを開始
        active_ = false;

        Signal::Params params;
        fw_.signal().sendMessage(Msg::START_TITLE_LOGO, params);
        return;
      }
      break;
    }

    spin_angle_ += spin_speed_ * delta_time;

    // 取得uirouの更新
    int cube_remain = static_cast<int>(rank_uirous_.size());
    float x = -cube_gap_.x() * (std::min(cube_remain, 4) - 1) / 2.0f;
    float y = 0.0f;
    float color = color_ease_(delta_time);
    int cube_num = 0;

    for (auto& uirou : rank_uirous_) {
      if (uirou.delay > 0.0f) {
        uirou.delay -= delta_time;
        if (uirou.ease_in && (uirou.delay <= 0.0f)) {
          // SE
          gamesound::play(fw_, "rank");
          uirou.ease_in = false;
        }
      }
      else if (uirou.scale_ease.isExec()) {
        uirou.scale = uirou.scale_ease(delta_time);
      }

      // ランクのういろうはメニューと同じ2D座標で指定されているので、それを3D座標に変換する
      Vec3f pos = Vec3f(x + cube_pos_.x(), y + cube_pos_.y(), cube_pos_.z());
      Vec3f cube_pos = menu::layoutPosToWorld(pos, cube_layout_, fw_.view(), camera_.projection(), Mat4f::Identity());
      
      uirou.matrix =
        Eigen::Translation<float, 3>(cube_pos)
        * Eigen::AngleAxisf(spin_angle_, Vec3f::UnitY())
        * spin_rotate_
        * Eigen::Scaling(Vec3f(cube_size_ * uirou.scale, cube_size_ * uirou.scale, cube_size_ * uirou.scale));

      Model::materialEmissiveColor(uirou.model, uirou.color * color);

      // テキストの表示位置
      uirou.text.pos(Vec2f(x + cube_pos_.x(), y + cube_pos_.y() + rank_text_ofs_));
      
      x += cube_gap_.x();

      // 二行以上を中央揃えで表示するための計算
      cube_remain -= 1;
      cube_num += 1;
      if (cube_num >= 4) {
        cube_num = 0;
        x  = -cube_gap_.x() * (std::min(cube_remain, 4) - 1) / 2.0f;
        y -= cube_gap_.y();
      }
    }
  }

  // 描画
  void draw(Signal::Params& arguments) {
    if (!updated_) return;
    
    auto prims = boost::any_cast<MatrixFont::PrimPack*>(arguments.at("text_prims"));

    if (!rank_uirous_.empty()) {
      pushMatrix();
      loadMatrix(Mat4f::Identity());

      for (auto& uirou : rank_uirous_) {
        modelDraw(uirou.model, uirou.matrix.matrix(), *shader_, true);

        uirou.text.draw(*prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f), font_mix_ - 10, 1);
      }
      
      popMatrix();
    }

    title_.draw(*prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f), font_mix_ - 28, 1);

    for (auto& t : text_) {
      t.draw(*prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f), font_mix_ - 20, 1);
    }
    
#if defined (USE_GAMECENTER)
    // GameCenter は状況で表示色を変更する
    GrpCol col = on_gamecenter_ ? GrpCol(1.0f, 0.0f, 0.0f, 1.0f) : GrpCol(1.0f, 1.0f, 1.0f, 1.0f);
    gamecenter_.draw(*prims, fw_.view(), col, font_mix_ - 4, 1);
    gamecenter_text_.draw(*prims, fw_.view(), col, font_mix_ - 10, 1);
#endif
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

  
};

}
