
#pragma once

//
// ゲーム内操作
//

#include "co_defines.hpp"
#include <Eigen/Geometry>
#include "co_quatEasing.hpp"
#include "nn_camera.hpp"
#include "nn_messages.hpp"
#include "nn_objBase.hpp"
#include "nn_signt.hpp"
#include "nn_touchRecord.hpp"


namespace ngs {

class Manipulate : public ObjBase, Touch::CallBack {
  Framework& fw_;
  const picojson::value& params_;
  Camera& camera_;
  
  bool active_;
  bool exec_;

  // 入力の記録、再生モード
  bool input_record_;
  bool input_playback_;

  u_int input_frame_;
  bool  first_frame_;

  TouchRecord touch_record_;
  float original_fovy_;

  // 回転操作
  EasingType::Type rotate_start_type_;
  EasingType::Type rotate_cont_type_;
  float rotate_start_time_;
  float rotate_cont_time_;
  float rotate_end_time_;
  float rotate_end_rate_;
  float rotate_rate_;

  QuatEasing rotate_ease_;
  Quatf      current_rotate_;
  
  // ピンチング操作
  EasingType::Type pinch_start_type_;
  EasingType::Type pinch_cont_type_;
  float pinch_start_time_;
  float pinch_cont_time_;
  Vec2f pinch_rate_;

  float current_z_;
  MiniEasing<float> pinch_ease_;

  float z_near_;
  float z_near_limit_;
  float z_far_;
  float z_far_limit_;

  float bound_;
  
  Quatf rotate_;
  Vec3f eye_;
  
  bool   touch_;
  u_long hash_;
  bool   pointed_;
  Vec3f  target_pos_;
  Vec2f  point_pos_;
  float  move_dist_limit_;

  float planet_radius_;

  bool drop_touch_;

  MiniEasing<float> view_effect_;
  float view_angle_;
  bool view_begin_;
  float camera_fovy_;

  
public:
  Manipulate(Framework& fw, const picojson::value& params,
             Camera& camera, const float planet_radius) :
    fw_(fw),
    params_(params.at("manipulate")),
    camera_(camera),
    active_(true),
    exec_(true),
    input_record_(false),
    input_playback_(false),
    input_frame_(0),
    rotate_start_type_(EasingType::fromString(params_.at("rotate_start_type").get<std::string>())),
    rotate_cont_type_(EasingType::fromString(params_.at("rotate_cont_type").get<std::string>())),
    rotate_start_time_(params_.at("rotate_start_time").get<double>()),
    rotate_cont_time_(params_.at("rotate_cont_time").get<double>()),
    rotate_end_time_(params_.at("rotate_end_time").get<double>()),
    rotate_end_rate_(params_.at("rotate_end_rate").get<double>()),
    rotate_rate_(params_.at("rotate_rate").get<double>()),
    pinch_start_type_(EasingType::fromString(params_.at("pinch_start_type").get<std::string>())),
    pinch_cont_type_(EasingType::fromString(params_.at("pinch_cont_type").get<std::string>())),
    pinch_start_time_(params_.at("pinch_start_time").get<double>()),
    pinch_cont_time_(params_.at("pinch_cont_time").get<double>()),
    pinch_rate_(vectFromJson<Vec2f>(params_.at("pinch_rate"))),
    z_near_(params_.at("z_near").get<double>()),
    z_near_limit_(params_.at("z_near_limit").get<double>()),
    z_far_(params_.at("z_far").get<double>()),
    z_far_limit_(params_.at("z_far_limit").get<double>()),
    bound_(params_.at("bound").get<double>()),
    rotate_(Quatf::Identity()),
    eye_(vectFromJson<Vec3f>(params_.at("eye"))),
    touch_(false),
    pointed_(false),
    move_dist_limit_(params_.at("move_dist").get<double>()),
    planet_radius_(planet_radius),
    drop_touch_(false),
    view_angle_(0.0f),
    view_begin_(false),
    camera_fovy_(camera.fovy())
  {
    DOUT << "Manipulate()" << std::endl;
    // タッチ入力のコールバックに登録
    fw_.touch().resistCallBack(this);

    camera_.eye()    = eye_;
    camera_.rotate() = rotate_;
  }

  ~Manipulate() {
    DOUT << "~Manipulate()" << std::endl;
    // タッチ入力のコールバックを解除
    fw_.touch().removeCallBack(this);
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
      // カメラの滑らか移動中は操作禁止
      DOUT << "Msg::START_EASE_CAMERA" << std::endl;

      exec_  = false;
      touch_ = false;

      rotate_ease_.stop();
      pinch_ease_.stop();
      endManipulateEffect();
      return;

    case Msg::END_EASE_CAMERA:
      // 滑らか移動が終わったらその状態をコピー
      DOUT << "Msg::END_EASE_CAMERA" << std::endl;
      eye_    = camera_.eye();
      rotate_ = camera_.rotate();
      exec_   = true;
      return;


    case Msg::FLASH_TOUCH_INPUT:
      touch_ = false;
      return;
      
    case Msg::MANIPULATE_ONCE_SKIP:
      DOUT << "Msg::MANIPULATE_ONCE_SKIP" << std::endl;
      pointed_    = false;
      drop_touch_ = true;
      return;

      
    case Msg::EXEC_DEMO_MODE:
      execDemoMode(arguments);
      return;
      
    case Msg::PAUSE_GAME:
      DOUT << "Msg::PAUSE_GAME" << std::endl;
      exec_  = false;
      touch_ = false;
      endManipulateEffect();
      return;

    case Msg::RESUME_GAME:
      DOUT << "Msg::RESUME_GAME" << std::endl;
      exec_ = true;
      return;

#if 0
    case Msg::ABORT_GAME:
      DOUT << "Msg::ABORT_GAME" << std::endl;
      return;
#endif

    case Msg::END_GAME:
      DOUT << "Msg::END_GAME" << std::endl;
      endGame(arguments);
      return;

      
    default:
      return;
    }
  }

  
  // タッチ操作のコールバック
  void touchStart(const Touch& touch, const std::vector<Touch::Info>& info) {
    if (touch_ || !exec_ || input_playback_) return;

    DOUT << "manupilate::touchStart" << std::endl;
    
    touch_     = true;
    hash_      = info[0].hash;
    point_pos_ = info[0].pos;
    pointed_   = drop_touch_ ? false : true;

    if (pointed_) {
      // あらかじめ照準位置を計算しておく
      pointed_ = Signt::calcSpawnPos(target_pos_, point_pos_, fw_.view(), camera_, planet_radius_);
    }
    
    DOUT << info[0].pos.x() << " " << info[0].pos.y() << std::endl;
  }
  
  void touchMove(const Touch& touch, const std::vector<Touch::Info>& info) {
    if (!touch_ || input_playback_) return;

    // const auto it = std::find(info.cbegin(), info.cend(), hash_);
    switch (info.size()) {
    case 1:
      if (info[0] == hash_) {
        beginManipulateEffect();
        
        // 原点を中心にカメラを回転
        Vec2f d_pos = info[0].pos - info[0].l_pos;
        float len   = d_pos.norm();

        // タッチ位置からの移動量が大きくなったら照準操作は取り消す
        if (pointed_) {
          Vec2f move_vec = info[0].pos - point_pos_;
          if (move_vec.norm() > move_dist_limit_) {
            pointed_ = false;
          }
        }
        
        if (len > 0.0f) {
          // タッチ座標と垂直なベクトル→回転ベクトル
          Vec3f vec = Vec3f(-d_pos.y(), d_pos.x(), 0.0f).normalized();

          // 回転角は、惑星上の座標に変換したタップ位置から算出
          float angle = angleOnCircle(lengthOnPlanet(len), planet_radius_) * rotate_rate_;

          // 球面線形補間で回転開始
          if (!rotate_ease_.isExec()) {
            current_rotate_ = Quatf(Eigen::AngleAxisf(angle, vec)) * rotate_;
            rotate_ease_.start(rotate_start_type_, rotate_start_time_, rotate_, current_rotate_);
          }
          else {
            current_rotate_ = Quatf(Eigen::AngleAxisf(angle, vec)) * current_rotate_;
            rotate_ease_.start(rotate_cont_type_, rotate_cont_time_, rotate_, current_rotate_);
          }
        }
      }
      break;

    case 2:
      if ((info[0] == hash_) || (info[1] == hash_)) {
        endManipulateEffect();

        // 照準操作は取り消す
        pointed_ = false;

        // ピンチイン・アウト
        Vec2f v   = info[0].pos - info[1].pos;
        Vec2f l_v = info[0].l_pos - info[1].l_pos;
        float d   = l_v.norm() - v.norm();

        // 限界距離では入力を減らす
        float z = pinch_ease_.isExec() ? current_z_ : eye_.z();
        float rate = 1.0f;
        if ((z < z_far_) && (d > 0.0f)) {
          rate = -1.0f / (z_far_limit_ - z_far_) * (z - z_far_) + 1.0f;
        }
        else if ((z > z_near_) && (d < 0.0f)) {
          rate = -1.0f / (z_near_limit_ - z_near_) * (z - z_near_) + 1.0f;
        }

        float pinch_distance = lengthOnPlanet(d);
        pinch_distance = pinch_distance * rate * ((pinch_distance < 0.0f) ? pinch_rate_(0) : pinch_rate_(1));
        
        if (!pinch_ease_.isExec()) {
          current_z_ = eye_.z() + pinch_distance;
          pinch_ease_.start(pinch_start_type_, pinch_start_time_, eye_.z(), current_z_);
        }
        else {
          current_z_ = current_z_ + pinch_distance;
          pinch_ease_.start(pinch_cont_type_, pinch_cont_time_, eye_.z(), current_z_);
        }
      }
      break;
    }
  }
  
  void touchEnd(const Touch& touch, const std::vector<Touch::Info>& info) {
    if (!touch_ || input_playback_) return;
    
    const auto it = std::find(info.cbegin(), info.cend(), hash_);
    if (it == info.cend()) return;

    if (rotate_ease_.isExec()) {
      // 慣性てきな動きを再現
      Vec2f d_pos = it->pos - it->l_pos;
      float len   = d_pos.norm() - 5.0f;

      if (len > 0.0f) {
        // タッチ座標と垂直なベクトル→回転ベクトル
        Vec3f vec = Vec3f(-d_pos.y(), d_pos.x(), 0.0f).normalized();

        // 回転角は、惑星上の座標に変換したタップ位置から算出
        float angle = angleOnCircle(lengthOnPlanet(len), planet_radius_) * rotate_end_rate_;

        // 球面線形補間で回転開始
        current_rotate_ = Quatf(Eigen::AngleAxisf(angle, vec)) * current_rotate_;
        rotate_ease_.start(rotate_cont_type_, rotate_end_time_, rotate_, current_rotate_);
      }
    }
        
    if (pointed_) {
      // タッチした位置からほとんど動かずに指を離したら攻撃開始
      DOUT << "Manipulate::pointed" << std::endl;
      startJumpAttack();
      pointed_ = false;
#ifdef _DEBUG
      if (input_record_) {
        touch_record_.recordTouch(input_frame_, target_pos_);
      }
#endif
    }

    endManipulateEffect();
    
    touch_ = false;
    DOUT << info[0].pos.x() << " " << info[0].pos.y() << std::endl;
  }


private:
  // 更新
  void update(const Signal::Params& arguments) {
#ifdef _DEBUG
    if (fw_.keyboard().getPushed() == 'C') {
      DOUT << "Camera Z:" << eye_.z() << std::endl;
    }
#endif

    // 入力の記録・再生
    input_frame_ += 1;
    if (input_record_) {
      touch_record_.recordCamera(input_frame_, camera_);
    }
    else if (input_playback_) {
      touch_record_.playbackCamera(input_frame_ - 1, camera_);
      if (touch_record_.playbackTouch(input_frame_ - 1, target_pos_)) {
        // FIXME:攻撃発動を直接呼び出している
        startJumpAttack();
      }
      else if (!touch_record_.canPlayback()) {
        // データが無くなった
        Signal::Params params;
        fw_.signal().sendMessage(Msg::PLAYBACK_FIN, params);
        input_playback_ = false;
      }
      return;
    }
    
    if (!exec_) return;

    float delta_time = boost::any_cast<float>(arguments.at("delta_time"));

    // 他の入力と被ったら操作を中断
    if (drop_touch_) {
      DOUT << "drop_touch" << std::endl;
      pointed_    = false;
      drop_touch_ = false;
    }
    
    // 回転をイージングで処理
    if (rotate_ease_.isExec()) {
      rotate_ = rotate_ease_(delta_time);
      camera_.rotate() = rotate_;
    }
    
    // ピンチングもイージングで処理
    if (pinch_ease_.isExec()) {
      eye_.z() = pinch_ease_(delta_time);
      camera_.eye() = eye_;
    }
    
    // 拡大・縮小の時に、指を離したら限界値でバウンドする処理
    if (!touch_) {
      if ((eye_.z() < z_far_) || (eye_.z() > z_near_)) {
        pinch_ease_.stop();
      }
      if (eye_.z() < z_far_) {
        float d = (eye_.z() - z_far_) * bound_;
        eye_.z() -= d;
        camera_.eye() = eye_;
      }
      else if (eye_.z() > z_near_) {
        float d = (eye_.z() - z_near_) * bound_;
        eye_.z() -= d;
        camera_.eye() = eye_;
      }
    }

    if (view_effect_.isExec()) {
      view_angle_ = view_effect_(delta_time);
      camera_.fovy(camera_fovy_ + view_angle_);
    }
  }


  // 操作演出開始
  void beginManipulateEffect() {
    if (view_begin_) return;
    
    view_effect_ = miniEasingFromJson<float>(params_.at("view_effect_begin"));
    view_effect_.degToRad();
    view_effect_.start(view_angle_);
    view_begin_ = true;
  }
  
  // 操作演出終了
  void endManipulateEffect() {
    if (!view_begin_) return;

    view_effect_ = miniEasingFromJson<float>(params_.at("view_effect_end"));
    view_effect_.degToRad();
    view_effect_.start(view_angle_);
    view_begin_ = false;
  }

  
  // 攻撃開始
  void startJumpAttack() {
    Signal::Params params;
    params.insert(Signal::Params::value_type("target_hash", createUniqueNumber()));
    params.insert(Signal::Params::value_type("target_pos", target_pos_));
    fw_.signal().sendMessage(Msg::START_JUMPATTACK, params);
  }

  
  // スクリーン上の長さを惑星上の長さに変換
	float lengthOnPlanet(const float length) const {
    // Screen Pos→Window Pos
    Vec2f win_pos = fw_.view().toWindowPos(Vec2f(length, 0.0f));

    Eigen::Affine3f m;
    m = Eigen::Translation<float, 3>(eye_);
    
    GLint view[4];
		glGetIntegerv(GL_VIEWPORT, view);

    // near平面とfar平面でのWorld座標を求める
    Vec3f pos_near;
		pointUnProject(pos_near, Vec3f(win_pos.x(), win_pos.y(), 0.0f), m.matrix(), camera_.projection(), view);
    Vec3f pos_far;
		pointUnProject(pos_far, Vec3f(win_pos.x(), win_pos.y(), 1.0f), m.matrix(), camera_.projection(), view);
    
    // 惑星表面上の値を計算
    float dt = (planet_radius_ - pos_near.z()) / (pos_far.z() - pos_near.z());
		return pos_near.x() + (pos_far.x() - pos_near.x()) * dt;
	}

  
  // ゲーム開始時の入力の記録・再生処理
  void execDemoMode(const Signal::Params& params) {
    // FIXME:排他的な機能はenumで表す??
    input_record_   = boost::any_cast<bool>(params.at("input_record"));
    input_playback_ = boost::any_cast<bool>(params.at("input_playback"));
    
    input_frame_ = 0;
    first_frame_ = true;
    
    if (input_record_) {
      Signal::Params param;
      fw_.signal().sendMessage(Msg::PLAYER_RECORD_INFO, param);
      
      touch_record_.startRecord(boost::any_cast<Quatf>(param.at("rotate")));
    }
    else if (input_playback_) {
      if (!touch_record_.readFromFile(fw_.savePath() + "touch_input.record")) {
        touch_record_.readFromFile(fw_.loadPath() + "touch_input.record");
      }
      Quatf rotate;
      touch_record_.startPlayback(rotate);

      Signal::Params param;
      param.insert(Signal::Params::value_type("rotate", rotate));
      fw_.signal().sendMessage(Msg::PLAYER_PLAYBACK_INFO, param);
      original_fovy_ = camera_.fovy();
    }
  }

  // ゲーム終了時の後始末
  void endGame(const Signal::Params& params) {
    touch_ = false;

    rotate_ease_.stop();
    pinch_ease_.stop();
    endManipulateEffect();

    // 入力の記録・再生処理の後始末
    if (input_record_) {
      touch_record_.writeToFile(fw_.savePath() + "touch_input.record");
    }
    else if (input_playback_) {
      camera_.fovy(original_fovy_);
    }
    
    input_record_   = false;
    input_playback_ = false;
  }
  
};

}
