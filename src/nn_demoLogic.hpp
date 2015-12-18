
#pragma once

//
// デモ進行
//


namespace ngs {

class DemoLogic : public ObjBase {
  Framework& fw_;
  const picojson::value& params_;

  bool active_;
  bool updated_;

  TextWidget demo_;

  enum class Mode {
    START,
    MAIN
  };
  Mode mode_;
  
  float delay_gamemain_;

  bool  demo_disp_;
  float demo_disp_time_;
  float demo_disp_speed_;

  
public:
  explicit DemoLogic(Framework& fw, const picojson::value& params,
                     const MatrixFont& font) :
    fw_(fw),
    params_(params.at("demo_logic")),
    active_(true),
    updated_(false),
    demo_(font, params_.at("demo")),
    mode_(Mode::START),
    delay_gamemain_(params_.at("delay_gamemain").get<double>()),
    demo_disp_(false),
    demo_disp_time_(0.0f),
    demo_disp_speed_(params_.at("demo_speed").get<double>())
  {
    DOUT << "DemoLogic()" << std::endl;

  }

  ~DemoLogic() {
    DOUT << "~DemoLogic()" << std::endl;
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

      
    case Msg::PLAYBACK_FIN:
      demoFin(arguments);
      return;

    case Msg::FINISH_SKIP_TAP:
      demoFin(arguments);
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

    demo_disp_time_ += delta_time;
    if (demo_disp_time_ > demo_disp_speed_) {
      demo_disp_ = !demo_disp_;
      demo_disp_time_ -= demo_disp_speed_;
    }

    switch (mode_) {
    case Mode::START:
      {
        // ゲーム本編開始待ち
        delay_gamemain_ -= delta_time;
        if (delay_gamemain_ <= 0.0f) {
          mode_ = Mode::MAIN;

          // ゲーム本編開始
          Signal::Params params;
          fw_.signal().sendMessage(Msg::START_GAMEMAIN, params);
          break;
        }
      }
      break;

    case Mode::MAIN:
      {
        // 再生データ終了待ち
      }
      break;
    }
  }
  
  void draw(Signal::Params& arguments) {
    if (!updated_) return;

    if (demo_disp_) {
      auto prims = boost::any_cast<MatrixFont::PrimPack*>(arguments.at("text_prims"));
      demo_.draw(*prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f));
    }
  }

  
  void demoFin(const Signal::Params& arguments) {
    // ゲーム本編強制終了
    Signal::Params params;
    fw_.signal().sendMessage(Msg::END_GAME, params);
  }
  
};

}
