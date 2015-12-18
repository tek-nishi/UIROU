
#pragma once

//
// ゲームの舞台となる惑星
//

#include <Eigen/Geometry>
#include "co_easyShader.hpp"
#include "co_modelDraw.hpp"
#include "co_misc.hpp"
#include "nn_modelHolder.hpp"
#include "nn_shaderHolder.hpp"
#include "nn_messages.hpp"
#include "nn_objBase.hpp"


namespace ngs {

class Planet : public ObjBase {
  Framework& fw_;
  const picojson::value& params_;
  
  bool active_;
  bool updated_;
  bool pause_;

  Model model_;
  std::shared_ptr<EasyShader> shader_;

  float scale_;

  
public:
  Planet(Framework& fw, const picojson::value& params,
         ModelHolder& model_holder, ShaderHolder& shader_holder,
         const float planet_radius) :
    fw_(fw),
    params_(params.at("planet")),
    active_(true),
    updated_(false),
    pause_(false),
    model_(model_holder.read(params_.at("model").get<std::string>())),
    shader_(shader_holder.read(params_.at("shader").get<std::string>())),
    scale_(planet_radius)
  {
    DOUT << "Planet()" << std::endl;
  }

  ~Planet() {
    DOUT << "~Planet()" << std::endl;
  }


  bool isActive() const {
    return active_;
  }
  
  void message(const int msg, Signal::Params& arguments) {
    switch (msg) {
    case Msg::UPDATE:
      update(arguments);
      return;

    case Msg::DRAW:
      draw(arguments);
      return;

      
    case Msg::PAUSE_GAME:
      pause_ = true;
      return;

    case Msg::RESUME_GAME:
      pause_ = false;
      return;

    case Msg::END_GAME:
      pause_ = false;
      return;

      
    default:
      return;
    }
  }


private:
  void update(const Signal::Params& arguments) {
    if (pause_) return;

    updated_ = true;
    
    // float delta_time = boost::any_cast<float>(arguments.at("delta_time"));
  }

  void draw(const Signal::Params& arguments) {
    if (!updated_) return;
    
    Eigen::Affine3f m;
    m = Eigen::Scaling(Vec3f(scale_, scale_, scale_));
    modelDraw(model_, m.matrix(), *shader_, true);
  }
  
};

}
