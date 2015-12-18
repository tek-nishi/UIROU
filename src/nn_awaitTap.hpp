
#pragma once

//
// 入力待ち
//

#include "nn_touchWidget.hpp"
#include "nn_textWidget.hpp"
#include "nn_menuMisc.hpp"


namespace ngs {

class AwaitTap : public ObjBase {
  Framework& fw_;
  const picojson::value& params_;
  TouchWidget& touch_widget_;

  bool active_;
  bool updated_;

  // 表示テキスト
  TextWidget text_;

  TouchWidget::Handle await_handle_;

  MiniEasing<float> mix_ease_;
  int font_mix_;
  
  bool on_await_;
  float await_time_;
  float blink_speed_;

  bool close_;

  // 色アニメ
  Ease<GrpCol> text_ease_color_;
  GrpCol       text_color_;

  bool exec_;
  bool paused_;
  

public:
  AwaitTap(Framework& fw, const picojson::value& params,
           const MatrixFont& font, TouchWidget& touch_widget) :
    fw_(fw),
    params_(params.at("await_tap")),
    touch_widget_(touch_widget),
    active_(true),
    updated_(false),
    text_(font, params_.at("text")),
    text_ease_color_(easeFromJson<GrpCol>(params_.at("text_color"))),
    mix_ease_(miniEasingFromJson<float>(params_.at("mix_in"))),
    on_await_(false),
    await_time_(0.0f),
    blink_speed_(params_.at("blink_speed").get<double>()),
    close_(false),
    exec_(false)
  {
    DOUT << "AwaitTap()" << std::endl;

    text_.text(params_.at("await_text").get(0).get<std::string>());

    // タッチ範囲のコールバックを登録
    await_handle_ = menu::addWidget(touch_widget_, text_,
                                    std::bind(&AwaitTap::awaitWidget,
                                              this, std::placeholders::_1, std::placeholders::_2), false);
  }

  ~AwaitTap() {
    DOUT << "~AwaitTap()" << std::endl;

    // タッチ範囲のコールバックを解除
    touch_widget_.removeWidget(await_handle_);
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


    case Msg::START_AWAIT_TAP:
      exec_ = true;
      touch_widget_.activeWidget(await_handle_, true);
      return;

      
    default:
      return;
    }
  }

  
private:
  // 更新
  void update(const Signal::Params& arguments) {
    updated_ = true;

    if (!exec_) return;
    
    float delta_time = boost::any_cast<float>(arguments.at("delta_time"));

    if (mix_ease_.isExec()) {
      font_mix_ = static_cast<int>(mix_ease_(delta_time));
      if (!mix_ease_.isExec() && close_) {
        // 終了
        active_ = false;
        return;
      }
    }

    if (close_) return;

    // テキストの明滅
    text_color_ = text_ease_color_(delta_time);

    // テキストアニメーション
    await_time_ += delta_time;

    float speed = blink_speed_;
    if (on_await_) speed *= 1.5f;

    const auto& text = params_.at("await_text").get<picojson::array>();
    text_.text(text[int(await_time_ * speed) % text.size()].get<std::string>());
  }

  // 描画
  void draw(const Signal::Params& arguments) {
    if (!updated_ || !exec_) return;

    // 描画プリミティブの格納先をポインタで受け取る
    auto prims = boost::any_cast<MatrixFont::PrimPack*>(arguments.at("text_prims"));

    GrpCol col = on_await_ ? GrpCol(1.0f, 0.0f, 0.0f, 1.0f) : text_color_;
    text_.draw(*prims, fw_.view(), col, font_mix_, 1);
  }

  
  void awaitWidget(const TouchWidget::Msg msg, const Vec2f& pos) {
    switch (msg) {
    case TouchWidget::TOUCH_START:
      {
        DOUT << "TOUCH_START" << std::endl;
        on_await_   = true;
        await_time_ = 0.0f;

        // 照準操作は中断
        Signal::Params params;
        fw_.signal().sendMessage(Msg::MANIPULATE_ONCE_SKIP, params);
      }
      break;
      
    case TouchWidget::MOVE_OUT_IN:
      DOUT << "MOVE_OUT_IN" << std::endl;
      on_await_   = true;
      await_time_ = 0.0f;
      break;
      
    case TouchWidget::MOVE_IN_OUT:
      DOUT << "MOVE_IN_OUT" << std::endl;
      on_await_ = false;
      text_.text(params_.at("await_text").get(0).get<std::string>());
      break;
      
    case TouchWidget::TOUCH_END_IN:
      DOUT << "TOUCH_END_IN" << std::endl;
      {
        // 入力待ち終了
        close_    = true;
        mix_ease_ = miniEasingFromJson<float>(params_.at("mix_out"));
        touch_widget_.activeWidget(await_handle_, false);
   
        Signal::Params param;
        fw_.signal().sendMessage(Msg::FINISH_AWAIT_TAP, param);      
      }
      break;

    case TouchWidget::TOUCH_END_OUT:
      DOUT << "TOUCH_END_OUT" << std::endl;
      on_await_ = false;
      text_.text(params_.at("await_text").get(0).get<std::string>());
      break;

    default:
      break;
    }
  }
  
};


}
