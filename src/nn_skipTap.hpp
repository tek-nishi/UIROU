
#pragma once

//
// 入力待ち(スキップ用)
//

namespace ngs {

class SkipTap : public ObjBase, Touch::CallBack {
  Framework& fw_;
  const picojson::value& params_;

  bool active_;
  bool updated_;

  bool  exec_;
  float exec_delay_;
  
  bool   touch_;
  u_long hash_;
  float  move_dist_;
  float  move_dist_limit_;
  bool   can_message_;


public:
  SkipTap(Framework& fw, const picojson::value& params) :
    fw_(fw),
    params_(params.at("skip_tap")),
    active_(true),
    updated_(false),
    exec_(false),
    exec_delay_(params_.at("exec_delay").get<double>()),
    touch_(false),
    move_dist_limit_(params_.at("move_dist_limit").get<double>())
  {
    DOUT << "SkipTap()" << std::endl;
    fw_.touch().resistCallBack(this);
  }

  ~SkipTap() {
    DOUT << "~SkipTap()" << std::endl;
    fw_.touch().removeCallBack(this);
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

      
    case Msg::ABORT_SKIP_TAP:
      touch_  = false;
      active_ = false;
      return;


    case Msg::FLASH_TOUCH_INPUT:
      {
        touch_ = false;
      }
      return;

      
    default:
      return;
    }
  }

  
  void touchStart(const Touch& touch, const std::vector<Touch::Info>& info) {
    // マルチタッチも無視
    if (!exec_ || touch_ || (info.size() != 1)) return;
    
    touch_       = true;
    hash_        = info[0].hash;
    move_dist_   = 0.0f;
    can_message_ = true;
  }
  
  void touchMove(const Touch& touch, const std::vector<Touch::Info>& info) {
    if (!touch_ || !can_message_) return;

    const auto it = std::find(info.cbegin(), info.cend(), hash_);
    if (it == info.cend()) return;
    
    // タップしてからの移動量を記録
    Vec2f d_pos = it->pos - it->l_pos;
    move_dist_ += d_pos.norm();
    if (move_dist_ > move_dist_limit_) {
      // タップ位置から動きすぎたら取り消し
      can_message_ = false;
    }
  }
  
  void touchEnd(const Touch& touch, const std::vector<Touch::Info>& info) {
    if (!touch_) return;

    const auto it = std::find(info.cbegin(), info.cend(), hash_);
    if (it == info.cend()) return;

    // タッチ終了時にイベント発生
    if (can_message_) {
      active_ = false;

      Signal::Params param;
      fw_.signal().sendMessage(Msg::FINISH_SKIP_TAP, param);
    }
    
    touch_ = false;
  }

  
private:
  void update(const Signal::Params& arguments) {
    updated_ = true;

    if (!exec_) {
      // スキップ操作開始まで若干待つ
      float delta_time = boost::any_cast<float>(arguments.at("delta_time"));
      exec_delay_ -= delta_time;
      if (exec_delay_ <= 0.0f) {
        exec_ = true;
      }
    }
  }
  
};

}
