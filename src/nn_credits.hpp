
#pragma once

//
// クレジット画面
//

#include "nn_gameSound.hpp"


namespace ngs {

class Credits : public ObjBase {
  Framework& fw_;
  const picojson::value& params_;

  bool active_;
  bool updated_;

  // 表示テキスト
  TextWidget title_;
  std::deque<TextWidget> text_;

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


public:
  Credits(Framework& fw, const picojson::value& params,
          const MatrixFont& font) :
    fw_(fw),
    params_(params.at("credits")),
    active_(true),
    updated_(false),
    title_(font, params_.at("title")),
    mode_(EFFECT_IN),
    mix_ease_(miniEasingFromJson<float>(params_.at("mix_in")))
  {
    DOUT << "Credits()" << std::endl;

    // 表示するテキストをコンテナに積む
    const picojson::array& text = params_.at("text").get<picojson::array>();
    for (const auto& value : text) {
      text_.emplace_back(font, value);
    }
  }

  ~Credits() {
    DOUT << "~Credits()" << std::endl;
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
          font_mix_ = 0;
          mode_ = TAP_DELAY;
          touch_exec_delay_ = 0.0f;          
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
      }
      break;
    }
  }

  // 描画
  void draw(Signal::Params& arguments) {
    if (!updated_) return;

    auto prims = boost::any_cast<MatrixFont::PrimPack*>(arguments.at("text_prims"));

    title_.draw(*prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f), font_mix_ - 40, 1);

    for (auto& t : text_) {
      t.draw(*prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f), font_mix_ - 35, 1);
    }
  }
  
};

}
