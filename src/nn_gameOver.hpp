
#pragma once

//
// ゲーム終了演出
//

#include "nn_textWidget.hpp"
#include "nn_easeInOut.hpp"


namespace ngs {

class GameOver : public ObjBase {
  Framework& fw_;
  const picojson::value& params_;

  bool active_;
  bool updated_;

  TextWidget title_;
  TextWidget sub_title_;

  EaseInOut<float> mix_ease_;
  int font_mix_;

  float duration_;
  
  
public:
  GameOver(Framework& fw, const picojson::value& params,
           const picojson::value& settings,
           const MatrixFont& font) :
    fw_(fw),
    params_(params.at("game_over")),
    active_(true),
    updated_(false),
    title_(font, params_.at("title")),
    sub_title_(font, params_.at("sub_title")),
    mix_ease_(params_.at("mix_in"), params_.at("mix_out"), 0.0f)
  {
    DOUT << "GameOver()" << std::endl;

    // 演出時間はプレイ回数が多いと短くなる
    MiniEasing<float> disp_time(miniEasingFromJson<float>(params_.at("disp_time")));
    float play_num = static_cast<float>(settings.at("number_of_played").get<double>());
    float duration = disp_time.at(play_num);

    gamesound::play(fw_, "game_over");
    
    mix_ease_.duration(duration);
    mix_ease_.start();

    duration_ = duration + static_cast<float>(params_.at("duration").get<double>());
  }

  ~GameOver() {
    DOUT << "~GameOver()" << std::endl;
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

      
    default:
      return;
    }
  }

  
private:
  void update(const Signal::Params& arguments) {
    updated_ = true;

    float delta_time = boost::any_cast<float>(arguments.at("delta_time"));

    // フォント表示演出
    font_mix_ = static_cast<int>(mix_ease_(delta_time));
    if (!mix_ease_.isExec()) {
      active_ = false;

      // 結果画面開始
      Signal::Params params;
      fw_.signal().sendMessage(Msg::START_GAME_RESULT, params);
      return;
    }

    // ゲーム終了待ち
    if (duration_ > 0.0f) {
      duration_ -= delta_time;
      if (duration_ <= 0.0f) {
        // 他のタスクにゲームオーバー画面終了を伝える
        Signal::Params params;
        fw_.signal().sendMessage(Msg::FINISH_GAME_OVER, params);
      }
    }
  }

  void draw(Signal::Params& arguments) {
    if (!updated_) return;

    // 描画プリミティブの格納先をポインタで受け取る
    auto prims = boost::any_cast<MatrixFont::PrimPack*>(arguments.at("text_prims"));

    // テキスト表示
    title_.draw(*prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f), font_mix_ - 30, 1);
    sub_title_.draw(*prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f), font_mix_ - 20, 1);
  }
  
};

}
