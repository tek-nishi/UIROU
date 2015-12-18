
#pragma once

//
// Mouse入力管理
//

namespace ngs {

class Mouse {
  // カーソル位置
  int x_;
  int y_;

  // ホイール位置
  int w_;

  // ボタンの状態
  bool l_push_;
  bool l_pull_;
  bool l_press_;

  bool r_push_;
  bool r_pull_;
  bool r_press_;

  bool m_push_;
  bool m_pull_;
  bool m_press_;

public:
  Mouse() :
    x_(0),
    y_(0),
    w_(0),
    l_push_(false),
    l_pull_(false),
    l_press_(false),
    r_push_(false),
    r_pull_(false),
    r_press_(false),
    m_push_(false),
    m_pull_(false),
    m_press_(false)
  {}

  // 内部情報を更新する
  void update(const int x, const int y, const int w,
              const bool l_press, const bool r_press, const bool m_press) {
    x_ = x;
    y_ = y;
    w_ = w;

    // ボタン情報の生成
    l_push_ = !l_press_ && l_press;
    l_pull_ = l_press_ && !l_press;
    l_press_ = l_press;
    
    r_push_ = !r_press_ && r_press;
    r_pull_ = r_press_ && !r_press;
    r_press_ = r_press;
    
    m_push_ = !m_press_ && m_press;
    m_pull_ = m_press_ && !m_press;
    m_press_ = m_press;
  }

  
  int x() const {
    return x_;
  }
  
  int y() const {
    return y_;
  }
  
  int w() const {
    return w_;
  }

  bool isLPush() const {
    return l_push_;
  }
  
  bool isLPull() const {
    return l_pull_;
  }
  
  bool isLPress() const {
    return l_press_;
  }
  
  bool isRPush() const {
    return r_push_;
  }
  
  bool isRPull() const {
    return r_pull_;
  }
  
  bool isRPress() const {
    return r_press_;
  }
  
  bool isMPush() const {
    return m_push_;
  }
  
  bool isMPull() const {
    return m_pull_;
  }
  
  bool isMPress() const {
    return m_press_;
  }
  
};

}
