
#pragma once

// 
// イージング
//

#include "co_defines.hpp"
#include <cmath>
#include <deque>
#include <algorithm>
#include <utility>
#include <map>


namespace ngs {

struct EasingType {
  enum Type {
    LINEAR,

    BACK_IN,
    BACK_OUT,
    BACK_INOUT,

    BOUNCE_IN,
    BOUNCE_OUT,
    BOUNCE_INOUT,

    CIRC_IN,
    CIRC_OUT,
    CIRC_INOUT,

    CUBIC_IN,
    CUBIC_OUT,
    CUBIC_INOUT,
	
    ELASTIC_IN,
    ELASTIC_OUT,
    ELASTIC_INOUT,
	
    EXPO_IN,
    EXPO_OUT,
    EXPO_INOUT,
	
    QUAD_IN,
    QUAD_OUT,
    QUAD_INOUT,
	
    QUART_IN,
    QUART_OUT,
    QUART_INOUT,
	
    QUINT_IN,
    QUINT_OUT,
    QUINT_INOUT,

    SINE_IN,
    SINE_OUT,
    SINE_INOUT
  };

  // EasingTypeとstd::stringから引く為の小さなクラス
  struct List {
  private:
    std::map<std::string, Type> tbl_;

  public:
    List() {
      // FIXME:VS2012で初期化子リストが使えないのでこう書く
      typedef std::pair<std::string, Type> TypeTbl;

      static const TypeTbl tbl[] = {
        std::make_pair("linear", LINEAR),
        std::make_pair("back_in", BACK_IN),
        std::make_pair("back_out", BACK_OUT),
        std::make_pair("back_inout", BACK_INOUT),

        std::make_pair("bounce_in", BOUNCE_IN),
        std::make_pair("bounce_out", BOUNCE_OUT),
        std::make_pair("bounce_inout", BOUNCE_INOUT),

        std::make_pair("circ_in", CIRC_IN),
        std::make_pair("circ_out", CIRC_OUT),
        std::make_pair("circ_inout", CIRC_INOUT),

        std::make_pair("cubic_in", CUBIC_IN),
        std::make_pair("cubic_out", CUBIC_OUT),
        std::make_pair("cubic_inout", CUBIC_INOUT),
	
        std::make_pair("elastic_in", ELASTIC_IN),
        std::make_pair("elastic_out", ELASTIC_OUT),
        std::make_pair("elastic_inout", ELASTIC_INOUT),
	
        std::make_pair("expo_in", EXPO_IN),
        std::make_pair("expo_out", EXPO_OUT),
        std::make_pair("expo_inout", EXPO_INOUT),
	
        std::make_pair("quad_in", QUAD_IN),
        std::make_pair("quad_out", QUAD_OUT),
        std::make_pair("quad_inout", QUAD_INOUT),
	
        std::make_pair("quart_in", QUART_IN),
        std::make_pair("quart_out", QUART_OUT),
        std::make_pair("quart_inout", QUART_INOUT),
	
        std::make_pair("quint_in", QUINT_IN),
        std::make_pair("quint_out", QUINT_OUT),
        std::make_pair("quint_inout", QUINT_INOUT),

        std::make_pair("sine_in", SINE_IN),
        std::make_pair("sine_out", SINE_OUT),
        std::make_pair("sine_inout", SINE_INOUT)
      };

      // あらかじめ用意された配列をコンテナに積む
      for (const auto& obj : tbl) {
        tbl_.insert(std::map<std::string, Type>::value_type(obj.first, obj.second));
      }
    }

    Type operator()(const std::string& type_name) const {
      return tbl_.at(type_name);
    }
    
  };

  // 文字列から EasingType を生成
  static Type fromString(const std::string& type_name) {
    // TIPS:最初に関数が飛び出された時にListが生成される
    static const List type_list;
    
    return type_list(type_name);
  }

};


template <typename T>
class EasingFunc {
//
// さまざまなイージング関数
// t = 現在の時間 b = 開始時の値 c = 変化量 d = 終了時間
//
  static T EasingLinear(float t, const T& b, const T& c, const float d) {
    return c*t/d + b;
  }

  
  static T EasingBackIn(float t, const T& b, const T& c, const float d) {
    float s = 1.70158f;
    t/=d;
    return c*t*t*((s+1)*t - s) + b;
  }

  static T EasingBackOut(float t, const T& b, const T& c, const float d) {
    float s = 1.70158f;
    t=t/d-1;
    return c*(t*t*((s+1)*t + s) + 1) + b;
  }

  static T EasingBackInOut(float t, const T& b, const T& c, const float d) {
    float s = 1.70158f * 1.525f;
    if ((t/=d/2) < 1) return c/2*(t*t*((s+1)*t - s)) + b;
    t-=2;
    return c/2*(t*t*((s+1)*t + s) + 2) + b;
  }


  static T EasingBounceOut(float t, const T& b, const T& c, const float d) {
    if ((t/=d) < (1/2.75f)) {
      return c*(7.5625*t*t) + b;
    } else if (t < (2/2.75f)) {
      t-=(1.5f/2.75f);
      return c*(7.5625f*t*t + .75f) + b;
    } else if (t < (2.5f/2.75f)) {
      t-=(2.25f/2.75f);
      return c*(7.5625f*t*t + .9375f) + b;
    } else {
      t-=(2.625f/2.75f);
      return c*(7.5625f*t*t + .984375f) + b;
    }
  }

  static T EasingBounceIn(float t, const T& b, const T& c, const float d) {
    // FIXME:引数に０を渡す苦肉の策
    return c - EasingBounceOut(d-t, T(b * 0), c, d) + b;
  }

  static T EasingBounceInOut(float t, const T& b, const T& c, const float d) {
    // FIXME:引数に０を渡す苦肉の策
    if (t < d/2) return EasingBounceIn(t*2, T(b * 0), c, d) * .5f + b;
    else         return EasingBounceOut(t*2-d, T(b * 0), c, d) * .5f + c*.5f + b;
  }


  static T EasingCircIn(float t, const T& b, const T& c, const float d) {
    t/=d;
    return -c * (std::sqrt(1 - t*t) - 1) + b;
  }

  static T EasingCircOut(float t, const T& b, const T& c, const float d) {
    t=t/d-1;
    return c * std::sqrt(1 - t*t) + b;
  }

  static T EasingCircInOut(float t, const T& b, const T& c, const float d) {
    if ((t/=d/2) < 1) return -c/2 * (std::sqrt(1 - t*t) - 1) + b;
    t-=2;
    return c/2 * (std::sqrt(1 - t*t) + 1) + b;
  }


  static T EasingCubicIn(float t, const T& b, const T& c, const float d) {
    t/=d;
    return c*t*t*t + b;
  }

  static T EasingCubicOut(float t, const T& b, const T& c, const float d) {
    t=t/d-1;
    return c*(t*t*t + 1) + b;
  }

  static T EasingCubicInOut(float t, const T& b, const T& c, const float d) {
    if ((t/=d/2) < 1) return c/2*t*t*t + b;
    t-=2;
    return c/2*(t*t*t + 2) + b;
  }


  static T EasingElasticIn(float t, const T& b, const T& c, const float d) {
    if (t==0)      return b;
    if ((t/=d)==1) return b+c;
    float p = d*0.3f;

    T a(c);
    float s=p/4;
    t-=1;
    return -(a * std::pow(2,10*t) * std::sin( (t*d-s)*(2 * m_pi)/p )) + b;
  }

  static T EasingElasticOut(float t, const T& b, const T& c, const float d) {
    if (t==0)      return b;
    if ((t/=d)==1) return b+c;
    float p = d*0.3f;

    T a(c);
    float s=p/4;
    return (a* std::pow(2,-10*t) * std::sin( (t*d-s)*(2 * m_pi)/p ) + c + b);
  }

  static T EasingElasticInOut(float t, const T& b, const T& c, const float d) {
    if (t==0) return b;
    if ((t/=d/2)==2) return b+c;
    float p = d*(0.3f*1.5f);

    T a(c);
    float s=p/4;
    if (t < 1)
    {
      t-=1;
      return -.5*(a * std::pow(2,10*t) * std::sin( (t*d-s)*(2 * m_pi)/p )) + b;
    }
    t-=1;
    return a * std::pow(2,-10*t) * std::sin( (t*d-s)*(2 * m_pi)/p )*0.5f + c + b;
  }


  static T EasingExpoIn(float t, const T& b, const T& c, const float d) {
    return (t==0) ? b : c * std::pow(2, 10 * (t/d - 1)) + b;
  }

  static T EasingExpoOut(float t, const T& b, const T& c, const float d) {
    return (t==d) ? T(b+c) : c * (-std::pow(2, -10 * t/d) + 1) + b;
  }

  static T EasingExpoInOut(float t, const T& b, const T& c, const float d) {
    if (t==0)         return b;
    if (t==d)         return b+c;
    if ((t/=d/2) < 1) return c/2 * std::pow(2, 10 * (t - 1)) + b;
    return c/2 * (-std::pow(2, -10 * --t) + 2) + b;
  }


  static T EasingQuadIn(float t, const T& b, const T& c, const float d) {
    t/=d;
    return c*t*t + b;
  }

  static T EasingQuadOut(float t, const T& b, const T& c, const float d) {
    t/=d;
    return -c *t*(t-2) + b;
  }

  static T EasingQuadInOut(float t, const T& b, const T& c, const float d) {
    if ((t/=d/2) < 1) return c/2*t*t + b;
    --t;
    return -c/2 * (t*(t-2) - 1) + b;
  }


  static T EasingQuartIn(float t, const T& b, const T& c, const float d) {
    t/=d;
    return c*t*t*t*t + b;
  }

  static T EasingQuartOut(float t, const T& b, const T& c, const float d) {
    t=t/d-1;
    return -c * (t*t*t*t - 1) + b;
  }

  static T EasingQuartInOut(float t, const T& b, const T& c, const float d) {
    if ((t/=d/2) < 1) return c/2*t*t*t*t + b;
    t-=2;
    return -c/2 * (t*t*t*t - 2) + b;
  }


  static T EasingQuintIn(float t, const T& b, const T& c, const float d) {
    t/=d;
    return c*t*t*t*t*t + b;
  }

  static T EasingQuintOut(float t, const T& b, const T& c, const float d) {
    t=t/d-1;
    return c*(t*t*t*t*t + 1) + b;
  }

  static T EasingQuintInOut(float t, const T& b, const T& c, const float d) {
    if ((t/=d/2) < 1) return c/2*t*t*t*t*t + b;
    t-=2;
    return c/2*(t*t*t*t*t + 2) + b;
  }


  static T EasingSineIn(float t, const T& b, const T& c, const float d) {
    return -c * std::cos(t/d * (m_pi/2)) + c + b;
  }

  static T EasingSineOut(float t, const T& b, const T& c, const float d) {
    return c * std::sin(t/d * (m_pi/2)) + b;
  }

  static T EasingSineInOut(float t, const T& b, const T& c, const float d) {
    return -c/2 * (std::cos(m_pi*t/d) - 1) + b;
  }

  T (* ease_func_)(float t, const T& b, const T& c, const float d);


  void setupFunc(const EasingType::Type type) {
    switch (type) {
		case EasingType::Type::BACK_IN:
      ease_func_ = EasingBackIn;
      break;

    case EasingType::Type::BACK_OUT:
      ease_func_ = EasingBackOut;
      break;

    case EasingType::Type::BACK_INOUT:
      ease_func_ = EasingBackInOut;
      break;

    case EasingType::Type::BOUNCE_IN:
      ease_func_ = EasingBounceIn;
      break;

    case EasingType::Type::BOUNCE_OUT:
      ease_func_ = EasingBounceOut;
      break;

    case EasingType::Type::BOUNCE_INOUT:
      ease_func_ = EasingBounceInOut;
      break;

    case EasingType::Type::CIRC_IN:
      ease_func_ = EasingCircIn;
      break;

    case EasingType::Type::CIRC_OUT:
      ease_func_ = EasingCircOut;
      break;

    case EasingType::Type::CIRC_INOUT:
      ease_func_ = EasingCircInOut;
      break;

    case EasingType::Type::CUBIC_IN:
      ease_func_ = EasingCubicIn;
      break;

    case EasingType::Type::CUBIC_OUT:
      ease_func_ = EasingCubicOut;
      break;

    case EasingType::Type::CUBIC_INOUT:
      ease_func_ = EasingCubicInOut;
      break;

    case EasingType::Type::ELASTIC_IN:
      ease_func_ = EasingElasticIn;
      break;

    case EasingType::Type::ELASTIC_OUT:
      ease_func_ = EasingElasticOut;
      break;

    case EasingType::Type::ELASTIC_INOUT:
      ease_func_ = EasingElasticInOut;
      break;

    case EasingType::Type::EXPO_IN:
      ease_func_ = EasingExpoIn;
      break;

    case EasingType::Type::EXPO_OUT:
      ease_func_ = EasingExpoOut;
      break;

    case EasingType::Type::EXPO_INOUT:
      ease_func_ = EasingExpoInOut;
      break;

    case EasingType::Type::QUAD_IN:
      ease_func_ = EasingQuadIn;
      break;

    case EasingType::Type::QUAD_OUT:
      ease_func_ = EasingQuadOut;
      break;

    case EasingType::Type::QUAD_INOUT:
      ease_func_ = EasingQuadInOut;
      break;

    case EasingType::Type::QUART_IN:
      ease_func_ = EasingQuartIn;
      break;

    case EasingType::Type::QUART_OUT:
      ease_func_ = EasingQuartOut;
      break;

    case EasingType::Type::QUART_INOUT:
      ease_func_ = EasingQuartInOut;
      break;

    case EasingType::Type::QUINT_IN:
      ease_func_ = EasingQuintIn;
      break;

    case EasingType::Type::QUINT_OUT:
      ease_func_ = EasingQuintOut;
      break;

    case EasingType::Type::QUINT_INOUT:
      ease_func_ = EasingQuintInOut;
      break;

    case EasingType::Type::SINE_IN:
      ease_func_ = EasingSineIn;
      break;

    case EasingType::Type::SINE_OUT:
      ease_func_ = EasingSineOut;
      break;

    case EasingType::Type::SINE_INOUT:
      ease_func_ = EasingSineInOut;
      break;

    default:
      ease_func_ = EasingLinear;
    }
  }

  
public:
  explicit EasingFunc(const EasingType::Type type) {
    setupFunc(type);
  }

  explicit EasingFunc(const std::string& type_name) {
    setupFunc(EasingType::fromString(type_name));
  }

  void type(const EasingType::Type type) {
    setupFunc(type);
  }
  

  // t 経過時間
  // b 開始値
  // c 最終値 - 開始値
  // d 持続時間
  T easing(float t, const T& b, const T& c, const float d) const {
    return ease_func_(t, b, c, d);
  }

	T operator()(float t, const T& b, const T& c, const float d) const {
    return ease_func_(t, b, c, d);
  }
  
};


template <typename T>
class Ease {
	class Object {
    EasingFunc<T> func_;
		T start_;
		T end_;
		float duration_;

	public:
		Object(const EasingType::Type type, const T& start, const T& end, const float duration) :
			func_(type),
			start_(start),
			end_(end),
			duration_(duration)
		{}

		Object(const Object& rhs) :
			func_(rhs.func_),
			start_(rhs.start_),
			end_(rhs.end_),
			duration_(rhs.duration_)
		{}

		T ease(const float time) const {
			float t = (time > 0.0f) ? ((time < duration_) ? time : duration_) : 0.0f;
			return func_.easing(t, start_, end_ - start_, duration_);
		}

		const T& start() const { return start_; }
		const T& end() const { return end_; }
		float duration() const { return duration_; }
	};

	std::deque<Object> objects_;
	float duration_;
	float start_;
	float time_;
	std::size_t current_;
  bool looping_;
	
public:
  explicit Ease(const bool looping = false) :
    duration_(0.0f),
    start_(0.0f),
    time_(0.0f),
		current_(0),
    looping_(looping)
  {}
  
	Ease(const EasingType::Type type, const T& start, const T& end, const float duration, const bool looping = false) :
		duration_(duration),
		start_(0.0f),
		time_(0.0f),
		current_(0),
    looping_(looping)
	{
		Object obj(type, start, end, duration);
		objects_.push_back(obj);
	}
  
	Ease(const std::string& type, const T& start, const T& end, const float duration, const bool looping = false) :
		duration_(duration),
		start_(0.0f),
		time_(0.0f),
		current_(0),
    looping_(looping)
	{
		Object obj(EasingType::fromString(type), start, end, duration);
		objects_.push_back(obj);
	}

	void add(const EasingType::Type type, const T& start, const T& end, const float duration) {
		Object obj(type, start, end, duration);
		objects_.push_back(obj);
		duration_ += duration;
	}

	void add(const std::string& type, const T& start, const T& end, const float duration) {
    add(EasingType::fromString(type), start, end, duration);
	}

	void add(const EasingType::Type type, const T& end, const float duration) {
    assert(!objects_.empty());
    
		const Object& last = objects_.back();
		Object obj(type, last.end(), end, duration);
		objects_.push_back(obj);
		duration_ += duration;
	}
  
	void add(const std::string& type, const T& end, const float duration) {
    add(EasingType::fromString(type), end, duration);
  }
  
  // 更新
	T operator()(const float delta_time) {
		time_ += delta_time;
		if (((time_ - start_) > objects_[current_].duration()) && (current_ < (objects_.size() - 1))) {
			start_ += objects_[current_].duration();
			++current_;
		}
    
    if (looping_ && isEnd()) {
      // 先頭に戻す
      time(time() - duration());
    }
    
		return objects_[current_].ease(time_ - start_);
	}

  
	void time(const float time) {
		float start = 0.0f;
		std::size_t idx;
		for (idx = 0; idx < (objects_.size() - 1); ++idx) {
			float duration = objects_[idx].duration();
			if (time < (duration + start)) break;
			start += duration;
		}
		time_    = time;
		current_ = idx;
		start_   = start;
	}

  void toStart() {
    time_    = 0.0f;
    start_   = 0.0f;
    current_ = 0;
  }
  
  void toEnd() {
    if (looping_) return;
    // looping時は終わりはないので処理をしない
    
    time_      = duration_;
    size_t idx = objects_.size() - 1;
    current_   = idx;
    start_     = duration_ - objects_[idx].duration();
  }

	float time() const { return time_; }
	// float start() const { return objects_.front().start(); }
	// float end() const { return objects_.back().end(); }
	float duration() const { return duration_; }

  void looping(const bool loop) { looping_ = loop; }
  bool isLoop() const { return looping_; }

  bool isEnd() const { return time_ >= duration_; }
  
};

}
