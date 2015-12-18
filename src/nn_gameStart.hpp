
#pragma once

//
// ゲーム開始演出
//

#include "nn_textWidget.hpp"
#include "nn_easeInOut.hpp"


namespace ngs {

class GameStart : public ObjBase {
  Framework& fw_;
  const picojson::value& params_;

  bool active_;
  bool updated_;

  TextWidget title_;
  TextWidget sub_title_;

  EaseInOut<float> mix_ease_;
  int font_mix_;
  
  
public:
  GameStart(Framework& fw, const picojson::value& params,
            const picojson::value& settings,
            const MatrixFont& font) :
    fw_(fw),
    params_(params.at("game_start")),
    active_(true),
    updated_(false),
    title_(font, params_.at("title")),
    sub_title_(font, params_.at("sub_title")),
    mix_ease_(params_.at("mix_in"), params_.at("mix_out"), 0.0f)
  {
    DOUT << "GameStart()" << std::endl;

    // 演出時間はプレイ回数が多いと短くなる
    MiniEasing<float> disp_time(miniEasingFromJson<float>(params_.at("disp_time")));
    float play_num = static_cast<float>(settings.at("number_of_played").get<double>());
    mix_ease_.duration(disp_time.at(play_num));
    mix_ease_.start();

    if (play_num < 5.0f) {
      // 開始時のカメラ演出(ある程度プレイしたら省略)
      Signal::Params camera_params;
      EaseCamera::setupFromJson(camera_params, params_.at("start_camera"));
      fw_.signal().sendMessage(Msg::START_EASE_CAMERA, camera_params);
    }

    gamesound::play(fw_, "game_start");
  }

  ~GameStart() {
    DOUT << "~GameStart()" << std::endl;
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

      // ゲーム本編開始
      Signal::Params params;
      fw_.signal().sendMessage(Msg::START_GAMEMAIN, params);
      return;
    }
  }

  void draw(Signal::Params& arguments) {
    if (!updated_) return;

    // 描画プリミティブの格納先をポインタで受け取る
    auto prims = boost::any_cast<MatrixFont::PrimPack*>(arguments.at("text_prims"));

    // テキスト生成
    title_.draw(*prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f), font_mix_ - 30, 1);
    sub_title_.draw(*prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f), font_mix_ - 20, 1);
  }
  
};

}
