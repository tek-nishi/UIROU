
#pragma once

//
// アプリ実行環境(GLKit)
// 実行に必要な情報を生成してAppに渡す
// 

#include "co_defines.hpp"
#include "nn_app.hpp"
#include "co_version.hpp"


namespace ngs {
namespace exec {

App*  app_instance = 0;
float screen_scale = 1.0f;


// タッチ座標→スクリーン座標
// TIPS:Objective-Cのコードが混在している
void createTouchInfo(std::vector<Touch::Info>& infoes, NSSet* const touches, UIView* const view, const float scale) {
  for (UITouch* touch in [touches allObjects]) {
    CGPoint pos_t   = [touch locationInView:view];
    CGPoint l_pos_t = [touch previousLocationInView:view];

    Vec2f pos   = app_instance->view().toScreenPos(pos_t.x * scale, pos_t.y * scale);
    Vec2f l_pos = app_instance->view().toScreenPos(l_pos_t.x * scale, l_pos_t.y * scale);
		
    Touch::Info info = {
      pos, l_pos, [touch hash]
    };
    infoes.push_back(info);
  }
}

// タッチ開始
void createTouchStartInfo(NSSet* const touches, UIView* const view) {
  std::vector<Touch::Info> infoes;
  createTouchInfo(infoes, touches, view, screen_scale);
  app_instance->touch().start(infoes);
}

// タッチ移動
void createTouchMoveInfo(NSSet* const touches, UIView* const view) {
  std::vector<Touch::Info> infoes;
  createTouchInfo(infoes, touches, view, screen_scale);
  app_instance->touch().move(infoes);
}

// タッチ終了
void createTouchEndInfo(NSSet* const touches, UIView* const view) {
  std::vector<Touch::Info> infoes;
  createTouchInfo(infoes, touches, view, screen_scale);
  app_instance->touch().end(infoes);
}


// 一時停止
void pause() {
  app_instance->pause();
}

// 再開
void resume() {
  app_instance->resume();
}


// キー入力
void keyInput(const int key_code) {
  app_instance->keyboard().updateCharaPush(key_code);
}


// アプリと実行環境の破棄
void destroy() {
  delete app_instance;
}


// アプリ環境の初期化
bool initialize(GLKView* view) {
  // 先にアプリを生成
  app_instance = new App;

  DOUT << "Program started." << std::endl;

  // OpenGL ES2.0 環境の生成
  EAGLContext* context = [[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2] autorelease];
  if (!context) {
    DOUT << "Failed to create ES2.0 context" << std::endl;
    destroy();
    return false;
  }

  // OpenGLのコンテキストをUIViewに紐づける
  view.context             = context;
  // view.drawableColorFormat = GLKViewDrawableColorFormatRGB565;
  view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
  [EAGLContext setCurrentContext:context];

  version::display();

  // 各機種で内部の座標系を統一する
  screen_scale        = [UIScreen mainScreen].scale;
  CGRect bounds       = view.bounds;
  int    screen_width = int(bounds.size.width * screen_scale);
  float  scale        = 1.0f;
  switch (screen_width) {
  case 320:
    // iPhone 3GS
    scale = 2.0f;
    break;

  case 1536:
    // iPad Retina
    scale = 0.5f;
    break;
  }
  
  // アプリに表示画面サイズを指示
  app_instance->view().setup(bounds.size.width, bounds.size.height, scale);

  return true;
}

// 更新
void update() {
  // 各種入力の更新
  app_instance->keyboard().update();

  // アプリの更新
  app_instance->update();
}

// 描画
void draw(const int width, const int height) {
  app_instance->view().resize(width, height);
  app_instance->draw();
}

}
}
