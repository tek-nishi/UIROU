
#pragma once

//
// オブジェクト間のメッセージング
//

#include <unordered_map>
#include <memory>
#include <functional>
#include <boost/noncopyable.hpp>
#include <boost/any.hpp>
#include <boost/signals2.hpp>


namespace ngs {

class Signal : private boost::noncopyable {
public:
  typedef std::unordered_map<std::string, boost::any> Params;
  typedef boost::signals2::connection Handle;

  
private:
#if 0
  // TIPS:実行を中断するためのカスタムコンバイナ
  struct Combiner {
    typedef bool result_type;
    template <typename InputIterator> result_type operator()(InputIterator aFirstObserver, InputIterator aLastObserver) const {
      result_type val = true;
      for (; aFirstObserver != aLastObserver && val; ++aFirstObserver)  {
        val = *aFirstObserver;            
      }
      return val;
    }
  };
  
  typedef boost::signals2::signal<bool (const int, Params&), Combiner> SignalType;
#endif

  typedef boost::signals2::signal<void (const int, Params&)> SignalType;
  
  SignalType signal_;

  
public:
  Signal() {
    DOUT << "Signal()" << std::endl;
  }

  ~Signal() {
    DOUT << "~Signal()" << std::endl;
  }

  
  template <typename T>
  void connect(std::shared_ptr<T> object) {
    // TIPS:メンバ関数に void message(const int) があれば何でも登録できる
    signal_.connect(SignalType::slot_type(&T::message, object.get(), _1, _2).track_foreign(object));
  }

  // shared_ptrでは無い場合
  // 切断は戻り値のメンバ関数disconnectで。
  template <typename T>
  Handle connect(T& object) {
    return signal_.connect(std::bind(&T::message, &object, std::placeholders::_1, std::placeholders::_2));
  }

  
  // 全オブジェクトにシグナル送信
  void sendMessage(const int msg, Signal::Params& arguments) {
    signal_(msg, arguments);
  }


  // 値があるか調べる
  static bool isParamValue(const Params& params, const std::string& name) {
    auto it = params.find(name);
    return it != params.cend();
  }
  
  // パラメーターの値を更新
  template <typename T>
  static void modifyParamValue(Params& params, const std::string& name, const T& value) {
    auto it = params.find(name);
    if (it == params.end()) {
      params.insert(Params::value_type(name, value));
    }
    else {
      it->second = boost::any_cast<T>(it->second) + value;
    }
  }  

  // パラメーターのコンテナに値を積む
  template <typename T>
  static void pushbackParamValue(Params& params, const std::string& name, const T& value) {
    auto it = params.find(name);
    if (it == params.end()) {
      std::deque<T> param;
      param.push_back(value);
      params.insert(Signal::Params::value_type(name, param));
    }
    else {
      std::deque<T>& param = boost::any_cast<std::deque<T>&>(it->second);
      param.push_back(value);
    }
  }
  
};

}
