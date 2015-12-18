
#pragma once

//
// ポーズメニュー表示
//

#include "co_defines.hpp"
#include <sstream>
#include <iomanip>
#include <functional>
#include "nn_touchWidget.hpp"
#include "nn_textWidget.hpp"
#include "nn_menuMisc.hpp"


namespace ngs {

class PauseMenu : public ObjBase {
  Framework& fw_;
  const picojson::value& params_;
  TouchWidget& touch_widget_;

  bool active_;
  bool updated_;

  TextWidget title_;
  TextWidget resume_;
  TextWidget abort_;

  MiniEasing<float> mix_ease_;
  int font_mix_;

  MiniEasing<float> volume_ease_;
  
  TouchWidget::Handle resume_handle_;
  TouchWidget::Handle abort_handle_;

  bool close_;
  enum Mode {
    RESUME,
    ABORT
  };
  Mode close_mode_;
  
  bool on_resume_;
  bool on_abort_;
  
  float resume_time_;
  float abort_time_;
  float blink_speed_;

  
public:
  PauseMenu(Framework& fw, const picojson::value& params,
            const MatrixFont& font, TouchWidget& touch_widget) :
    fw_(fw),
    params_(params.at("pause_menu")),
    touch_widget_(touch_widget),
    active_(true),
    updated_(false),
    title_(font, params_.at("title")),
    resume_(font, params_.at("resume")),
    abort_(font, params_.at("abort")),
    mix_ease_(miniEasingFromJson<float>(params_.at("mix_in"))),
    volume_ease_(miniEasingFromJson<float>(params_.at("volume_in"))),
    close_(false),
    on_resume_(false),
    on_abort_(false),
    resume_time_(0.0f),
    abort_time_(0.0f),
    blink_speed_(params_.at("blink_speed").get<double>())
  {
    DOUT << "PauseMenu()" << std::endl;

    abort_.text(params_.at("abort_text").get(0).get<std::string>());
    resume_.text(params_.at("resume_text").get(0).get<std::string>());

    gamesound::play(fw_, "pause");
    
    // タッチ範囲のコールバックを登録
    resume_handle_ = menu::addWidget(touch_widget_, resume_,
                                     std::bind(&PauseMenu::resumeWidget,
                                               this, std::placeholders::_1, std::placeholders::_2), true);

    abort_handle_ = menu::addWidget(touch_widget_, abort_,
                                     std::bind(&PauseMenu::abortWidget,
                                               this, std::placeholders::_1, std::placeholders::_2), true);
  }

  ~PauseMenu() {
    DOUT << "~PauseMenu()" << std::endl;
    
    // タッチ範囲のコールバックを解除
    touch_widget_.removeWidget(resume_handle_);
    touch_widget_.removeWidget(abort_handle_);
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


    case Msg::END_GAME:
      active_ = false;
      return;
      
      
    default:
      return;
    }
  }

  
private:
  void update(const Signal::Params& arguments) {
    updated_ = true;

    float delta_time = boost::any_cast<float>(arguments.at("delta_time"));

    // Fontの表示演出
    if (mix_ease_.isExec()) {
      font_mix_ = static_cast<int>(mix_ease_(delta_time));
      if (!mix_ease_.isExec() && close_) {
        switch (close_mode_) {
        case RESUME:
          {
            // ポーズ解除
            Signal::Params param;
            fw_.signal().sendMessage(Msg::RESUME_GAME, param);
          }
          break;

        case ABORT:
          {
            // ゲーム中断
            Signal::Params param;
            fw_.signal().sendMessage(Msg::END_GAME, param);
          }
          break;
        }
        active_ = false;
        return;
      }
    }

    // BGMの音量を変更
    if (volume_ease_.isExec()) {
      float volume = volume_ease_(delta_time);
      fw_.sound().gain("bgm", volume);
    }

    if (close_) return;

    // テキストアニメーション
    {
      resume_time_ += delta_time;
      float speed = blink_speed_;
      if (on_resume_) speed *= 1.5f;
    
      const auto& text = params_.at("resume_text").get<picojson::array>();
      resume_.text(text[int(resume_time_ * speed) % text.size()].get<std::string>());
    }

    {
      abort_time_ += delta_time;
      float speed = blink_speed_;
      if (on_abort_) speed *= 1.5f;

      const auto& text = params_.at("abort_text").get<picojson::array>();
      abort_.text(text[int(abort_time_ * speed + 2.0f) % text.size()].get<std::string>());
    }
    
  }

  void draw(Signal::Params& arguments) {
    if (!updated_) return;

    // 描画プリミティブの格納先をポインタで受け取る
    auto prims = boost::any_cast<MatrixFont::PrimPack*>(arguments.at("text_prims"));

    // メニュー表示
    title_.draw(*prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f), font_mix_ - 21, 1);

    {
      // ボタンはタップ中色を変える
      GrpCol col = on_resume_ ? GrpCol(1.0f, 0.0f, 0.0f, 1.0f) : GrpCol(1.0f, 1.0f, 1.0f, 1.0f);
      resume_.draw(*prims, fw_.view(), col, font_mix_ - 18, 1);
    }
    {
      // ボタンはタップ中色を変える
      GrpCol col = on_abort_ ? GrpCol(1.0f, 0.0f, 0.0f, 1.0f) : GrpCol(1.0f, 1.0f, 1.0f, 1.0f);
      abort_.draw(*prims, fw_.view(), col, font_mix_ - 15, 1);
    }
  }


  void resumeWidget(const TouchWidget::Msg msg, const Vec2f& pos) {
    switch (msg) {
    case TouchWidget::TOUCH_START:
      DOUT << "TOUCH_START" << std::endl;
      on_resume_   = true;
      resume_time_ = 0.0f;
      break;
      
    case TouchWidget::MOVE_OUT_IN:
      DOUT << "MOVE_OUT_IN" << std::endl;
      on_resume_   = true;
      resume_time_ = 0.0f;
      break;
      
    case TouchWidget::MOVE_IN_OUT:
      DOUT << "MOVE_IN_OUT" << std::endl;
      on_resume_ = false;
      resume_.text(params_.at("resume_text").get(0).get<std::string>());
      break;
      
    case TouchWidget::TOUCH_END_IN:
      DOUT << "RESUME" << std::endl;
      gamesound::play(fw_, "pause");
      closeMenu(RESUME);
      break;

    case TouchWidget::TOUCH_END_OUT:
      DOUT << "TOUCH_END_OUT" << std::endl;
      on_resume_ = false;
      resume_.text(params_.at("resume_text").get(0).get<std::string>());
      break;

    default:
      break;
    }
  }
  
  void abortWidget(const TouchWidget::Msg msg, const Vec2f& pos) {
    switch (msg) {
    case TouchWidget::TOUCH_START:
      DOUT << "TOUCH_START" << std::endl;
      on_abort_   = true;
      abort_time_ = 0.0f;
      break;
      
    case TouchWidget::MOVE_OUT_IN:
      DOUT << "MOVE_OUT_IN" << std::endl;
      on_abort_   = true;
      abort_time_ = 0.0f;
      break;
      
    case TouchWidget::MOVE_IN_OUT:
      DOUT << "MOVE_IN_OUT" << std::endl;
      on_abort_ = false;
      abort_.text(params_.at("abort_text").get(0).get<std::string>());
      break;
      
    case TouchWidget::TOUCH_END_IN:
      DOUT << "BACK TO TITLE" << std::endl;
      gamesound::stopAll(fw_);
      gamesound::play(fw_, "wipe");
      closeMenu(ABORT);
      {
        Signal::Params param;
        fw_.signal().sendMessage(Msg::ABORT_GAME, param);
      }
      break;

    case TouchWidget::TOUCH_END_OUT:
      DOUT << "TOUCH_END_OUT" << std::endl;
      on_resume_ = false;
      abort_.text(params_.at("abort_text").get(0).get<std::string>());
      break;

    default:
      break;
    }
  }


  // メニューを閉じる演出開始
  void closeMenu(const Mode mode) {
    close_      = true;
    close_mode_ = mode;
        
    touch_widget_.activeWidget(resume_handle_, false);
    touch_widget_.activeWidget(abort_handle_, false);

    mix_ease_    = miniEasingFromJson<float>(params_.at("mix_out"));
    volume_ease_ = miniEasingFromJson<float>(params_.at("volume_out"));
  }
  
};

}
