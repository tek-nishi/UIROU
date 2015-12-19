
#pragma once

//
// アプリ実行環境(GLFW)
// 実行に必要な情報を生成してAppに渡す
//

#include "co_defines.hpp"

// OpenGL関連のインクルードは順番がある…
#if defined (_MSC_VER)
#include <windows.h>
#include <GL/glew.h>
#endif
#include <GL/glfw.h>
#if defined (__APPLE__)
#include <OpenGL/glext.h>
#endif
#include "co_glext.hpp"
#include "nn_app.hpp"
#include "co_version.hpp"


// リンクするライブラリの指示(Windows)
#if defined (_MSC_VER)
#pragma comment(lib, "GLFWDLL.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#ifdef _DEBUG
#pragma comment(lib, "glew32sd.lib")
#else
#pragma comment(lib, "glew32s.lib")
#endif
#endif


namespace ngs {
namespace exec {

App* app_instance = 0;

// タッチ情報生成用
Vec2f pos_l_;
Vec2f m_pos_l_;
bool  touch_on_;
bool  muitl_touch_on_;
bool  shift_on_;


// タッチ開始・終了座標を内部座標系に変換してTouchに渡す
void GLFWCALL mouseButtonCallback(const int button, const int action) {
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    touch_on_ = (action == GLFW_PRESS) ? true : false;

    if (action == GLFW_PRESS) {
      // SHIFTを押した状態で左クリックしたらピンチイン・アウトを真似る
      muitl_touch_on_ = (glfwGetKey(GLFW_KEY_LSHIFT) == GLFW_PRESS) ||
                        (glfwGetKey(GLFW_KEY_RSHIFT) == GLFW_PRESS);

      // CTRLを押した状態で左クリックしたら二本指シフトを真似る
      shift_on_ = (glfwGetKey(GLFW_KEY_LCTRL) == GLFW_PRESS) ||
                  (glfwGetKey(GLFW_KEY_RCTRL) == GLFW_PRESS);
    }

    // マウスカーソルの座標→スクリーン座標
    int x, y;
    glfwGetMousePos(&x, &y);
    Vec2f pos(app_instance->view().toScreenPos(x, y));
    
    Vec2f m_pos;
    if (muitl_touch_on_ || shift_on_) {
      // マルチタッチを真似る場合は原点対称の座標を使う
      m_pos = -pos;
    }
    else {
      m_pos = pos;
    }

    // タッチ情報を配列で用意してからvectorに格納
    Touch::Info info[2] = {
      { pos, pos_l_, 0 },
      { m_pos, m_pos_l_, 1 }
    };

    std::vector<Touch::Info> touches;
    touches.push_back(info[0]);
    if (muitl_touch_on_ || shift_on_) touches.push_back(info[1]);

    if (action == GLFW_PRESS) {
      app_instance->touch().start(touches);
    }
    else {
      app_instance->touch().end(touches);
      muitl_touch_on_ = false;
      shift_on_ = false;
    }
		
    pos_l_ = pos;
    m_pos_l_ = m_pos;
  }
}

// タッチ座標が移動した時に内部座標系に変換してTouchに渡す
void GLFWCALL mouseMoveCallback(const int x, const int y) {
  if (!touch_on_) return;
	
  // マウスカーソルの座標→スクリーン座標
  Vec2f pos(app_instance->view().toScreenPos(x, y));
  Vec2f m_pos;

  if (muitl_touch_on_) {
    m_pos = -pos;
  }
  else if (shift_on_) {
    Vec2f d = pos - pos_l_;
    m_pos = m_pos_l_ + d;
  }
    
  Touch::Info info[2] = {
    { pos, pos_l_, 0 },
    {	m_pos, m_pos_l_, 1 }
  };

  std::vector<Touch::Info> touches;
  touches.push_back(info[0]);
  if (muitl_touch_on_ || shift_on_) touches.push_back(info[1]);

  app_instance->touch().move(touches);

  pos_l_ = pos;
  m_pos_l_ = m_pos;
}


// 画面サイズの変更
void GLFWCALL changeWindowSize(const int width, const int height) {
  app_instance->view().resize(width, height);
}


// キー入力の更新
int keyConvGlfwCode(const int key) {
  struct KeyConv {
    int from;
    int to;
  };
  static const KeyConv tbl[] = {
    { GLFW_KEY_ESC, Keyboard::ESC },

    { GLFW_KEY_F1,  Keyboard::F1 },
    { GLFW_KEY_F2,  Keyboard::F2 },
    { GLFW_KEY_F3,  Keyboard::F3 },
    { GLFW_KEY_F4,  Keyboard::F4 },
    { GLFW_KEY_F5,  Keyboard::F5 },
    { GLFW_KEY_F6,  Keyboard::F6 },
    { GLFW_KEY_F7,  Keyboard::F7 },
    { GLFW_KEY_F8,  Keyboard::F8 },
    { GLFW_KEY_F9,  Keyboard::F9 },
    { GLFW_KEY_F10, Keyboard::F10 },

    { GLFW_KEY_UP,     Keyboard::UP },    
    { GLFW_KEY_DOWN,   Keyboard::DOWN },    
    { GLFW_KEY_LEFT,   Keyboard::LEFT },    
    { GLFW_KEY_RIGHT,  Keyboard::RIGHT },
    { GLFW_KEY_LSHIFT, Keyboard::SHIFT },
    { GLFW_KEY_RSHIFT, Keyboard::SHIFT },
    { GLFW_KEY_LCTRL,  Keyboard::CTRL },
    { GLFW_KEY_RCTRL,  Keyboard::CTRL },
    { GLFW_KEY_LALT,   Keyboard::ALT },
    { GLFW_KEY_RALT,   Keyboard::ALT },
    { GLFW_KEY_TAB,    Keyboard::TAB },
    { GLFW_KEY_ENTER,  Keyboard::ENTER },
    { GLFW_KEY_DEL,    Keyboard::DEL },
  };

  for (const auto& it : tbl) {
    if (it.from == key) return it.to;
  }
  return key;
}

void GLFWCALL createKeyInfo(const int key, const int state) {
  const int key_code = (key < GLFW_KEY_SPECIAL) ? key : keyConvGlfwCode(key);
  switch (state) {
  case GLFW_PRESS:
    app_instance->keyboard().updateKeyPush(key_code);
    if (key >= GLFW_KEY_SPECIAL) {
      // 英文字以外の入力を記録
      app_instance->keyboard().updateCharaPush(key_code);
    }
    break;
    
  case GLFW_RELEASE:
    app_instance->keyboard().updateKeyRelease(key_code);
    break;
  }
}

void GLFWCALL createCharaInfo(const int key, const int state) {
  // 英文字以外は無視
  if (key >= GLFW_KEY_SPECIAL) return;
    
  switch (state) {
  case GLFW_PRESS:
    app_instance->keyboard().updateCharaPush(key);
    break;
  }
}


// マウスの更新
void updateMouse() {
  int x, y;
  glfwGetMousePos(&x, &y);

  int w = glfwGetMouseWheel();

  bool l_press = glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
  bool r_press = glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
  bool m_press = glfwGetMouseButton(GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
  
  app_instance->mouse().update(x, y, w, l_press, r_press, m_press);
}


#if 0

// ゲームパッドの更新
std::vector<u_int> gamepad_id;
std::vector<Gamepad::Info> gamepad_info;

void initGamepad() {
  // 接続されているゲームパッドを登録
  for (u_int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; ++i) {
    if (glfwGetJoystickParam(i, GLFW_PRESENT) == GL_TRUE) {
      gamepad_id.push_back(i);

      Gamepad::Info info = {
        glfwGetJoystickParam(i, GLFW_BUTTONS),
        glfwGetJoystickParam(i, GLFW_AXES)
      };
      gamepad_info.push_back(info);

      DOUT << "Pad" << info.button_num << " " << info.axes_num << std::endl;
    }
  }
  Env::gamepad().setup(gamepad_info);
}

void updateGamepad() {
  for (u_int i = 0; i < gamepad_id.size(); ++i) {
    std::vector<u_char> buttons(gamepad_info[i].button_num);
    glfwGetJoystickButtons(gamepad_id[i], &buttons[0], gamepad_info[i].button_num);

    std::vector<float> axes(gamepad_info[i].axes_num);
    glfwGetJoystickPos(gamepad_id[i], &axes[0], gamepad_info[i].axes_num);
    
    Env::gamepad().update(i, buttons, axes);
  }
}

#endif


// アプリと実行環境の破棄
void destroy() {
  // コールバック停止
  glfwSetWindowSizeCallback(0);
  glfwSetKeyCallback(0);
  glfwSetCharCallback(0);
  glfwSetMouseButtonCallback(0);
  glfwSetMousePosCallback(0);

  // AppのデストラクタでOpenGLを使っているので先に破棄
  delete app_instance;

  glfwTerminate();

  DOUT << "Program finished." << std::endl;
}

// 実行環境の初期化
bool initialize() {
  // GLFWの初期化
  if (glfwInit() != GL_TRUE) {
    DOUT << "glfwInit() error!" << std::endl;
    return false;
  }

  // アプリのインスタンスを生成
  // std::coutでデバッグ出力できるようになる(Windows)
  app_instance = new App;

  DOUT << "Program started." << std::endl;

  // OpenGLのコンテキストの初期化はglfwOpenWindow()で行われる
  int width  = App::WIDTH;
  int height = App::HEIGHT;
  if (glfwOpenWindow(width, height,
                     0, 0, 0, 0,
                     24, 0,
                     App::FULLSCREEN ? GLFW_FULLSCREEN : GLFW_WINDOW) != GL_TRUE) {

    DOUT << "glfwOpenWindow() error!" << std::endl;
    destroy();
    return false;
  }

  version::display();

  // Appに描画範囲を指示
  app_instance->view().setup(width, height, 1.0);
  
  // バッファ更新とモニタ更新の同期を取る
  glfwSwapInterval(1);

  // OpenGL拡張の初期化
  if (!initGlExt()) {
    DOUT << "Can't Execute glext code" << std::endl;
#if defined (_MSC_VER)
    // Windows版は対応していない機種が多いので警告を表示する
    MessageBox(NULL, "This app requires OpenGL 2.1 or later.", "Error", MB_OK);
#endif
    destroy();
    return false;
  }

  glfwSetWindowTitle(PREPRO_TO_STR(PRODUCT_NAME));

  // コールバック登録
  glfwSetWindowSizeCallback(changeWindowSize);
  glfwSetKeyCallback(createKeyInfo);
  glfwSetCharCallback(createCharaInfo);
  glfwSetMouseButtonCallback(mouseButtonCallback);
  glfwSetMousePosCallback(mouseMoveCallback);

#if 0
  // 入力の初期化
  initGamepad();
#endif
  
  return true;
}

// 実行中か判定
bool isActive() {
  return glfwGetWindowParam(GLFW_OPENED);
}

// 更新
void update() {
  // 各種入力の更新
  updateMouse();
  // updateGamepad();
  app_instance->keyboard().update();

  // アプリの更新
  app_instance->update();
}

// 描画
void draw() {
  app_instance->draw();

  // 表示バッファを入れ替えて描画を実行
  glfwSwapBuffers();
}

}

}
