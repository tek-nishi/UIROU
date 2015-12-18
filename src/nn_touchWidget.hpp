
#pragma once

//
// メニューのタッチ処理
//

#include "co_defines.hpp"
#include <list>
#include <functional>
#include "co_view.hpp"


namespace ngs {

class TouchWidget : Touch::CallBack {
public:
  enum Msg {
    NONE,
    
		TOUCH_START,                                    // 枠内でタッチ
		MOVE_OUT_IN,                                    // 枠外→枠内
		MOVE_IN,                                        // 枠内でドラッグ
		MOVE_IN_OUT,                                    // 枠内→枠外
		MOVE_OUT,                                       // 枠外でドラッグ
		TOUCH_END_IN,                                   // 枠内で離された
		TOUCH_END_OUT,                                  // 枠外で離された
  };
  
  typedef std::function<void (const Msg, const Vec2f&)> CallBack;
  
  struct Widget {
    CallBack call_back;
    Vec2f pos;
    Vec2f size;
    View::Layout layout;
    bool active;
  };

  typedef std::list<Widget>::iterator Handle;

  
private:
  Framework& fw_;

  std::list<Widget> widgets_;
  
  bool   touch_;
  u_long touch_hash_;
  bool   touch_in_;
  Handle touch_handle_;

  // 枠内だと判定する余白
  Vec2f margin_;
  
  
public:
  explicit TouchWidget(Framework& fw) :
    fw_(fw),
    touch_(false),
    margin_(0.0f, 0.0f)
  {
    DOUT << "TouchWidget()" << std::endl;

    fw_.touch().resistCallBack(this);
  }

  ~TouchWidget() {
    DOUT << "~TouchWidget()" << std::endl;

    fw_.touch().removeCallBack(this);
  }


  void margin(const float margin) {
    margin_ << margin, margin;
  }

  
  Handle addWidget(CallBack call_back, const Vec2f& pos, const Vec2f& size, const View::Layout layout) {
    Widget widget = {
      call_back, pos, size, layout, true
    };
    // 末尾に登録して、そのイテレーターを返す
    return widgets_.insert(widgets_.end(), widget);
  }

  void removeWidget(const Handle handle) {
    // 操作中の場合は、操作を無効にする
    if (touch_ && (handle == touch_handle_)) {
      touch_ = false;
    }
    widgets_.erase(handle);
  }

  void activeWidget(Handle handle, const bool active) {
    handle->active = active;

    // 操作中のWidgetをpauseする場合
    if (!active && touch_ && (handle == touch_handle_)) {
      touch_ = false;
    }
  }

  void flashTouch() {
    if (touch_) {
      // 動作中のコールバックは「範囲外で終了」扱い
      touch_handle_->call_back(Msg::TOUCH_END_OUT, Vec2f::Zero());
      touch_ = false;
    }
  }

  bool isTouching() const { return touch_; }

  
  void touchStart(const Touch& touch, const std::vector<Touch::Info>& info) {
    if (touch_ || (info.size() > 1)) return;

    for (auto it = widgets_.begin(); it != widgets_.end(); ++it) {
      if (it->active && testPointWidget(info[0].pos, margin_, *it)) {
        touch_        = true;
        touch_hash_   = info[0].hash;
        touch_in_     = true;
        touch_handle_ = it;

        it->call_back(Msg::TOUCH_START, info[0].pos);
        
        break;
      }
    }
  }

  void touchMove(const Touch& touch, const std::vector<Touch::Info>& info) {
    if (!touch_) return;

    const auto it = std::find(info.cbegin(), info.cend(), touch_hash_);
    if (it == info.cend()) return;

    bool move_in = testPointWidget(it->pos, margin_ * 1.5f, *touch_handle_);
    Msg msg = Msg::NONE;
    if (move_in) {
      if (touch_in_) {
        // 枠内でタッチ位置が動いた
        msg = Msg::MOVE_IN;
      }
      else {
        // 枠外→枠内へ移動
        msg = Msg::MOVE_OUT_IN;
      }
    }
    else {
      if (touch_in_) {
        // 枠内→枠外へ移動
        msg = Msg::MOVE_IN_OUT;
      }
      else {
        // 枠外でタッチ位置が動いた
        msg = Msg::MOVE_OUT;
      }
    }
    assert(msg != Msg::NONE);
    touch_handle_->call_back(msg, it->pos);
    
    touch_in_ = move_in;
  }

  void touchEnd(const Touch& touch, const std::vector<Touch::Info>& info) {
    if (!touch_) return;

    const auto it = std::find(info.cbegin(), info.cend(), touch_hash_);
    if (it == info.cend()) return;

    Msg msg = Msg::NONE;
    if (testPointWidget(it->pos, margin_ * 1.5f, *touch_handle_)) {
      // 枠内でタッチ終了
      msg = Msg::TOUCH_END_IN;
    }
    else {
      // 枠外でタッチ終了
      msg = Msg::TOUCH_END_OUT;
    }
    assert(msg != Msg::NONE);
    touch_handle_->call_back(msg, it->pos);
    
    touch_ = false;
  }

  
private:
  bool testPointWidget(const Vec2f& pos, const Vec2f& margin, const Widget& widget) {
    Vec2f l_pos(pos.x(), pos.y());

    Vec2f inf = fw_.view().layoutPos(widget.pos, widget.layout);
    Vec2f sup(inf);

    // 画面では上方向がプラスなので、矩形の座標もそのように用意
    inf.y() -= widget.size.y();
    sup.x() += widget.size.x();

    Box box = { inf - margin, sup + margin };
#if 0
    DOUT << l_pos.x() << "," << l_pos.y() << " "
         << box.inf.x() << "," << box.inf.y() << " "
         << box.sup.x() << "," << box.sup.y() << std::endl;
#endif
    
    return testPointBox(l_pos, box);
  }

  
};

}
