
#pragma once

//
// ゲーム内サウンド管理
// FIXME:シングルトンを止める
//

#include <set>
#include "co_json.hpp"
#include "co_framework.hpp"
#include "co_vector.hpp"


namespace ngs {
namespace gamesound {

// BGM/SE振り分け
enum Type {
  NONE,
  BGM,
  SE
};
std::map<std::string, Type> types_;

std::set<std::string> bgm_category_;
std::set<std::string> se_category_;

// ON/OFF設定
bool active_bgm_ = true;
bool active_se_  = true;


// jsonからサウンドデータの準備
std::deque<Sound::Info> readFromJson(const picojson::value& params, const std::string& load_path) {
  std::deque<Sound::Info> infoes;
  
  const auto& values = params.get<picojson::array>();
  for (const auto& value : values) {
    const std::string& name     = value.at("name").get<std::string>();
    const std::string& category = value.at("category").get<std::string>();

      // パラメーターから生成情報を作る
    Sound::Info info = {
      name,
      load_path + value.at("file").get<std::string>(),
      category,
      static_cast<float>(value.at("gain").get<double>()),
      value.at("loop").get<bool>()
    };
    infoes.push_back(info);
    
    // BGMかSEかを登録
    const std::string& type = value.at("type").get<std::string>();

    Type sound_type = NONE;
    if (type == "bgm") {
      bgm_category_.insert(category);
      sound_type = BGM;
    }
    else if (type == "se") {
      se_category_.insert(category);
      sound_type = SE;
    }
    assert(sound_type != NONE);
    
    types_.insert(std::unordered_map<std::string, Type>::value_type(name, sound_type));
  }
    
  return infoes;
}

// BGMとSEのON/OFF設定
void active(const bool bgm, const bool se) {
  active_bgm_ = bgm;
  active_se_  = se;
}

bool isActive(const Type type) {
  switch (type) {
  case BGM:
    return active_bgm_;

  case SE:
    return active_se_;

  default:
    assert(0);
    return false;
  }
}


// 再生
void play(Framework& fw, const std::string& name, const Vec3f& pos = Vec3f::Zero()) {
  Type type = types_.at(name);
  switch(type) {
  case BGM:
    if (!active_bgm_) return;
    break;

  case SE:
    if (!active_se_) return;
    break;

  default:
    break;
  }
  
  fw.sound().play(name, pos);
}

// 停止(BGM or SE)
void stop(Framework& fw, const Type type) {
  switch (type) {
  case BGM:
    for (const auto& category : bgm_category_) {
      fw.sound().stop(category);
    }
    return;

  case SE:
    for (const auto& category : se_category_) {
      fw.sound().stop(category);
    }
    return;

  default:
    break;
  }
}

// 停止(カテゴリ指定版)
void stop(Framework& fw, const std::string& category) {
  fw.sound().stop(category);
}

// 全停止
void stopAll(Framework& fw) {
  stop(fw, BGM);
  stop(fw, SE);
}

}
}
