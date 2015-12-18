
#pragma once

//
// OpenGLの状態を監視
//

#include "co_defines.hpp"


namespace {

class GlState {
  bool blend_;
  bool depth_test_;
  bool cull_face_;


public:
  GlState() :
    blend_(false),
    depth_test_(false),
    cull_face_(false)
  {}


  void blend(const bool flag) {
    if (flag != blend_) {
      if (flag) glEnable(GL_BLEND);
      else      glDisable(GL_BLEND);

      blend_ = flag;
    }
  }
  
  void depthTest(const bool flag) {
    if (flag != depth_test_) {
      if (flag) glEnable(GL_DEPTH_TEST);
      else      glDisable(GL_DEPTH_TEST);

      depth_test_ = flag;
    }
  }
  
  void cullFace(const bool flag) {
    if (flag != cull_face_) {
      if (flag) glEnable(GL_CULL_FACE);
      else      glDisable(GL_CULL_FACE);

      cull_face_ = flag;
    }
  }
  
};

}
