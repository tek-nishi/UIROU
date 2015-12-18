
#pragma once

// 
// カメラの滑らか移動
//

#include "co_easing.hpp"
#include "co_miniEasing.hpp"
#include "co_quatEasing.hpp"


namespace ngs {

class EaseCamera : public ObjBase {
  Framework& fw_;
  Camera& camera_;
  
  bool active_;

  // 距離のイージング用
  MiniEasing<float> distance_;

  // 向きのイージング用
  QuatEasing rotate_;

  
public:
  EaseCamera(Framework& fw, Camera& camera) :
    fw_(fw),
    camera_(camera),
    active_(true)
  {
    DOUT << "EaseCamera()" << std::endl;
  }

  ~EaseCamera() {
    DOUT << "~EaseCamera()" << std::endl;
  }


  bool isActive() const {
    return active_;
  }
  
  void message(const int msg, Signal::Params& arguments) {
    switch (msg) {
    case Msg::UPDATE:
      update(arguments);
      return;

      
    case Msg::START_EASE_CAMERA:
      start(arguments);
      return;

    case Msg::TO_END_EASE_CAMERA:
      distance_.toEnd();
      rotate_.toEnd();
      return;

      
    default:
      return;
    }
  }
  

private:
  void update(Signal::Params& arguments) {
    if (!distance_.isExec()) return;

    float delta_time = boost::any_cast<float>(arguments.at("delta_time"));

    // 注視点からの距離
    camera_.eye() = Vec3f(0.0f, 0.0f, distance_(delta_time));

    // 向きは球面補完で処理
    camera_.rotate() = rotate_(delta_time);

    if (!distance_.isExec()) {
      // カメラの指示を終えた後で終了シグナルを送る
      fw_.signal().sendMessage(Msg::END_EASE_CAMERA, arguments);
    }
  }

  void start(const Signal::Params& arguments) {
    const Vec3f& eye = camera_.eye();

    const auto& type     = boost::any_cast<const std::string&>(arguments.at("type"));
    const auto  duration = boost::any_cast<float>(arguments.at("duration"));
    
    distance_.start(type,
                    duration,
                    eye.z(), boost::any_cast<float>(arguments.at("end_z")));

    rotate_.start(type,
                  duration,
                  camera_.rotate(), boost::any_cast<Quatf>(arguments.at("end_rot")));
  }


public:
  // カメラの滑らか移動セットアップ
  static void setupFromJson(Signal::Params& params, const picojson::value& json) {
    params.insert(Signal::Params::value_type("type", json.at("type").get<std::string>()));

    params.insert(Signal::Params::value_type("duration",
                                             float(json.at("duration").get<double>())));
    params.insert(Signal::Params::value_type("end_z",
                                             float(json.at("end_z").get<double>())));
    params.insert(Signal::Params::value_type("end_rot",
                                             quatFromJson(json.at("end_rot"))));
  }
  
};

}
