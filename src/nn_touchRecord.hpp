
#pragma once

//
// 操作の記録・再生
//

#include <fstream>
#include <vector>
#include <deque>
#include "co_touch.hpp"


namespace ngs {

class TouchRecord {
  struct CameraInfo {
    u_int frame;
    float fovy;
    Vec3f eye;
    Quatf rotate;
  };
  std::deque<CameraInfo> camera_info_;
  size_t camera_index_;
  
  struct TouchInfo {
    u_int frame;
    Vec3f pos;
  };
  std::deque<TouchInfo> touch_info_;
  size_t touch_index_;

  Quatf player_rotate_;
  
  

public:
  TouchRecord() {
  }

  ~TouchRecord() {
  }


  void startRecord(const Quatf& rotate) {
    camera_info_.clear();
    touch_info_.clear();

    player_rotate_ = rotate;
  }

  void startPlayback(Quatf& rotate) {
    camera_index_ = 0;
    touch_index_  = 0;

    rotate = player_rotate_;
  }

  void recordCamera(const u_int frame, const Camera& camera) {
    CameraInfo info = {
      frame,
      camera.fovy(), camera.eye(), camera.rotate()
    };
    camera_info_.push_back(info);
  }

  void recordTouch(const u_int frame, const Vec3f& pos) {
    TouchInfo info = {
      frame,
      pos
    };
    touch_info_.push_back(info);
  }
  
  void writeToFile(const std::string& fullpath) {
    std::ofstream fs(fullpath);

    if (!fs) {
      DOUT << "file open error!: " << fullpath << std::endl;
      return;
    }

    fs << player_rotate_.x() << " " << player_rotate_.y() << " " << player_rotate_.z() << " " << player_rotate_.w()
       << std::endl;
    
    fs << camera_info_.size() << std::endl;
    
    for (const auto& info : camera_info_) {
      fs << info.frame << " ";

      fs << info.fovy << " ";
      
      const Vec3f eye = info.eye;
      fs << eye.x() << " " << eye.y() << " " << eye.z() << " ";

      const Quatf& rotate = info.rotate;
      fs << rotate.x() << " " << rotate.y() << " " << rotate.z() << " " << rotate.w() << std::endl;
    }
    
    fs << touch_info_.size() << std::endl;
    for (const auto& info : touch_info_) {
      fs << info.frame << " ";
      
      const Vec3f pos = info.pos;
      fs << pos.x() << " " << pos.y() << " " << pos.z() << std::endl;
    }
  }

  void playbackCamera(const u_int frame, Camera& camera) {
    if (camera_index_ == camera_info_.size()) return;
    if (frame < camera_info_[camera_index_].frame) return;

    camera.fovy(camera_info_[camera_index_].fovy);
    camera.eye()    = camera_info_[camera_index_].eye;
    camera.rotate() = camera_info_[camera_index_].rotate;
    
    ++camera_index_;
  }

  bool canPlayback() const {
    return (touch_index_ != touch_info_.size());
  }
  
  bool playbackTouch(const u_int frame, Vec3f& touch_pos) {
    if (touch_index_ == touch_info_.size()) return false;
    if (frame < touch_info_[touch_index_].frame) return false;

    touch_pos = touch_info_[touch_index_].pos;
    
    ++touch_index_;

    return true;
  }

  bool readFromFile(const std::string& fullpath) {
    std::ifstream fs(fullpath);

    if (!fs) {
      DOUT << "file open error!: " << fullpath << std::endl;
      return false;
    }

    fs >> player_rotate_.x() >> player_rotate_.y() >> player_rotate_.z() >> player_rotate_.w();

    camera_info_.clear();
    size_t camera_num;
    fs >> camera_num;
    while (camera_num > 0) {
      CameraInfo info;
      fs >> info.frame
         >> info.fovy
         >> info.eye.x() >> info.eye.y() >> info.eye.z()
         >> info.rotate.x() >> info.rotate.y() >> info.rotate.z() >> info.rotate.w();

      camera_info_.push_back(info);
      
      --camera_num;
    }

    touch_info_.clear();
    size_t touch_num;
    fs >> touch_num;
    while (touch_num > 0) {
      TouchInfo info;
      fs >> info.frame
         >> info.pos.x() >> info.pos.y() >> info.pos.z();

      touch_info_.push_back(info);
      
      --touch_num;
    }
    
    return true;
  }

  
private:

  
};

}
