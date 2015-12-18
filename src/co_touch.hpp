
#pragma once

// 
// タッチ入力
//

#include "co_defines.hpp"
#include <iostream>
#include <list>
#include <vector>
#include <functional>
#include <algorithm>
#include <boost/noncopyable.hpp>
#include "co_vector.hpp"


namespace ngs {

class Touch : private boost::noncopyable {
public:
  struct Info {
    // 現在の位置
    Vec2f pos;
    // １つ前の位置
    Vec2f l_pos;
    // マルチタッチを見分けるハッシュ値
    u_long hash;

    bool operator ==(const u_long rhs_hash) const {
      return hash == rhs_hash;
    }
  };

  struct CallBack {
  protected:
    ~CallBack() {}
    // TIPS:delete (TouchCallBack *)hoge; の禁止

  public:
    virtual void touchStart(const Touch& touch, const std::vector<Info>& info) = 0;
    virtual void touchMove(const Touch& touch, const std::vector<Info>& info) = 0;
    virtual void touchEnd(const Touch& touch, const std::vector<Info>& info) = 0;
  };

  
private:
  std::list<CallBack*> cb_;

  
public:
	Touch() {
		DOUT << "Touch()" << std::endl;
	}

	~Touch() {
		DOUT << "~Touch()" << std::endl;
	}

  
	// コールバック登録
	void resistCallBack(CallBack* const cb) {
    // 二重登録禁止
    assert(std::find(cb_.begin(), cb_.end(), cb) == cb_.end());
		cb_.push_back(cb);
	}

	// コールバック解除
	void removeCallBack(const CallBack* const cb) {
    auto it = std::find(cb_.begin(), cb_.end(), cb);
    assert(it != cb_.end());
    cb_.erase(it);
	}

	
	template <class T, void (T::*F)(const Touch& touch, const std::vector<Info>& info)>
	struct callback {
		const Touch& touch_;
		const std::vector<Info>& info_;
		
		callback(const Touch& touch, const std::vector<Info>& info) :
			touch_(touch),
			info_(info)
		{}

		void operator()(CallBack* const func) const {
			(func->*F)(touch_, info_);
			// TIPS:メンバ関数をテンプレート引数として扱う
		}
	};

  // タッチ開始
	void start(const std::vector<Info>& info) {
		std::for_each(cb_.begin(), cb_.end(),
                  callback<CallBack, &CallBack::touchStart>(*this, info));
	}

  // タッチ位置の移動
	void move(const std::vector<Info>& info) {
		std::for_each(cb_.begin(), cb_.end(),
                  callback<CallBack, &CallBack::touchMove>(*this, info));
	}

  // タッチ終了
	void end(const std::vector<Info>& info) {
		std::for_each(cb_.begin(), cb_.end(),
                  callback<CallBack, &CallBack::touchEnd>(*this, info));
	}
	
};

}
