
#pragma once

//
// アプリ本体
//

#include <memory>
#include "co_framework.hpp"
#include "nn_gameProc.hpp"


namespace ngs {

class App : public Framework {
#ifdef _DEBUG
  bool pause_;
#endif
  
  bool proc_paused_;
  
  // 安定するまでは一定時間で動作させる
  int stability_fame_;
  // 現在の時間
  double current_time_;
  
  // 実行中のメインルーチン
  std::unique_ptr<ProcBase> proc_;

  
public:
  enum {
    WIDTH      = 960,
    HEIGHT     = 640,
    FULLSCREEN = 0,
    FRAMERATE  = 60
  };

  
  App() :
#ifdef _DEBUG
    pause_(false),
#endif
    proc_paused_(false),
    stability_fame_(5),
    current_time_(0.0)
  {
    DOUT << "App()" << std::endl;
  }

  ~App() {
    DOUT << "~App()" << std::endl;
  }

  
  void update() {
    if (proc_paused_) return;

    // 起動直後は実行時間一定で更新する
    if (stability_fame_ > 0) --stability_fame_;
    
    // 直前の実行時間を次のフレームの経過時間とする
    double next_time  = time().current();
    double delta_time = (stability_fame_ > 0) ? (1.0 / App::FRAMERATE) : (next_time - current_time_);
    current_time_ = next_time;

#ifdef _DEBUG
    bool force_exec = false;
    
    if (pause_ && keyboard().isPush('.')) {
      // コマ送り
      force_exec = true;
    }
    else if (keyboard().isPress('.')) {
      // 早送り
      delta_time = (1.0 / App::FRAMERATE) * 4.0;
    }
    else if (keyboard().isPress(',')) {
      // スロー再生
      delta_time = (1.0 / App::FRAMERATE) / 8.0;
    }
    if (proc_ && (keyboard().getPushed() == 'R')) {
      // ソフトリセット
      proc_.reset();
    }
#endif
    
    if (!proc_) {
      // コンストラクタ内ではまだOpenGLのコンテキストが準備されていないので
      // メインロジックはここで生成
      proc_ = std::unique_ptr<ProcBase>(new GameProc(*this));
    }

#ifdef _DEBUG
    // 更新の一時停止判定
    if (keyboard().isPush(Keyboard::ESC)) pause_ = !pause_;
    if (pause_ && !force_exec) return;
#endif

    proc_->update(delta_time);
  }
  
  void draw() {
   if (proc_) proc_->draw();
  }

  // GameCanterやTweet画面表示中にアプリの実行を止める
  void pause() {
    proc_paused_ = true;
    if (proc_) proc_->pause();
  }

  // アプリの動作再開
  void resume() {
    if (proc_) proc_->resume();

    // iOSの画面切り替えなどで動作が安定するまでは強制1/60モード
    stability_fame_ = 5;
    proc_paused_    = false;
  }
  
};

}
