
#pragma once

//
// Keyboard入力管理
//

#include <unordered_map>
#include <vector>


namespace ngs {

class Keyboard {

public:

  // 英数字以外は以下の値を使って判定
  enum {
    ESC = 256,

    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,

    UP,    
    DOWN,    
    LEFT,    
    RIGHT,
    SHIFT,
    CTRL,
    ALT,
    TAB,
    ENTER,
    DEL
  };

private:
  std::unordered_map<int, bool> press_keys_;
  std::vector<std::unordered_map<int, bool> > push_keys_;
  std::vector<std::unordered_map<int, bool> > pull_keys_;
  std::vector<int> push_chara_;

  int page_;
  
public:
  Keyboard() :
    push_keys_(2),
    pull_keys_(2),
    page_(0)
  {
    push_chara_.push_back(0);
    push_chara_.push_back(0);
  }


  // キー更新。システムからキーを渡す
  void updateKeyPush(const int key) {
    press_keys_[key] = true;
    push_keys_[page_ ^ 1][key] = true;
  }
  
  void updateKeyRelease(const int key) {
    press_keys_[key] = false;
    pull_keys_[page_ ^ 1][key] = true;
  }

  void updateCharaPush(const int chara) {
    push_chara_[page_ ^ 1] = chara;
  }


  // 内部情報の更新。
  void update() {
    push_keys_[page_].clear();
    pull_keys_[page_].clear();
    push_chara_[page_] = 0;
    page_ = page_ ^ 1;
  }
  

  // キー情報の取得。英文字は大文字のみ有効
  // SHIFTを押しながらの入力は見分けられない
  // TODO:引数は小文字でも大丈夫にする…？
  bool isPush(const int key) const {
    const auto it = push_keys_[page_].find(key);
    if (it == push_keys_[page_].cend()) return false;
    return it->second;
  }

  bool isPull(const int key) const {
    const auto it = pull_keys_[page_].find(key);
    if (it == pull_keys_[page_].cend()) return false;
    return it->second;
  }

  bool isPress(const int key) const {
    const auto it = press_keys_.find(key);
    if (it == press_keys_.cend()) return false;
    return it->second;
  }

  // ダイアログなどで使う、通常のキー入力
  // SHIFTを押しながらの入力も見分ける
  int getPushed() const {
    return push_chara_[page_];
  }
  
};

}
