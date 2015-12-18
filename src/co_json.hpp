
#pragma once

//
// Json 読み書き
//

#include <picojson.h>
#include "co_vector.hpp"
#include "co_easing.hpp"
#include "co_miniEasing.hpp"
#include "co_zlib.hpp"


namespace ngs {

// Json読み込み
picojson::value readJson(const std::string& filename) {
  std::ifstream fstr(filename);
  if (fstr) {
    // ファイルからjsonオブジェクトを生成
    picojson::value json;
    fstr >> json;
    return json;
  }
  DOUT << "No file:" << filename << std::endl;
  return picojson::value();
}

// Json書き出し
void writeJson(const picojson::value& json, const std::string& filename) {
  std::ofstream fstr(filename);
  if (fstr) {
    fstr << json;
  }
}


// zlibで圧縮されたファイルから読み込む
picojson::value readJsonZlib(const std::string& filename) {
  // zlib圧縮されたデータをコンテナに展開
  std::vector<char> output;
  output.reserve(1024);
  zlib::read(filename, output);

  // 読み込み用ストリームを生成
  std::istringstream packed_stream(std::string(&output[0], output.size()));

  // jsonへ変換
  picojson::value json;
  packed_stream >> json;

  return json;
}

// Jsonをzlibで圧縮して書き出し
void writeJsonZlib(const picojson::value& json, const std::string& filename) {
  // jsonをストリームに変換
  std::ostringstream sstr;
  sstr << json;

  // ストリームをコンテナに展開
  const std::string& str = sstr.str();
  std::vector<char> output(str.begin(), str.end());

  // zlib圧縮して書き出し
  zlib::write(filename, output);
}


// 配列からベクトルを生成
template <typename T>
T vectFromJson(const picojson::value& json) {
  const auto& array = json.get<picojson::array>();

  T vec;
  for (int i = 0; i < vec.rows(); ++i) {
    vec(i) = array[i].get<double>();
  }
  return vec;
}

// 配列からクオータニオンを生成
Quatf quatFromJson(const picojson::value& json) {
  const auto& array = json.get<picojson::array>();

  Quatf q(Eigen::AngleAxisf(deg2rad(array[0].get<double>()),
                            Vec3f(array[1].get<double>(), array[2].get<double>(), array[3].get<double>()).normalized()));
  return q;
}

// 値を加えて更新(int, float, double)
template <typename T>
void modifyAddJson(picojson::value& json, const std::string& name, const T value) {
  // 書き換える場所
  picojson::value& param = json.at(name);

  // 元の値に加える
  T v = static_cast<T>(param.get<double>());
  param = picojson::value(static_cast<double>(v + value));
}

// イージングを生成
template <typename T>
Ease<T> easeFromJson(const picojson::value& json) {
  Ease<T> ease(json.at("loop").get<bool>());

  const auto& values = json.at("values").get<picojson::array>();
  for (const auto& value : values) {
    if (value.get("start").is<picojson::array>()) {
      // 開始値と終了値を指定
      ease.add(value.at("type").get<std::string>(),
               vectFromJson<T>(value.at("start")), vectFromJson<T>(value.at("end")),
               value.at("duration").get<double>());
    }
    else {
      // 終了値のみの指定
      ease.add(value.at("type").get<std::string>(),
               vectFromJson<T>(value.at("end")),
               value.at("duration").get<double>());
    }
  }

  return ease;
}

template <>
Ease<float> easeFromJson(const picojson::value& json) {
  Ease<float> ease(json.at("loop").get<bool>());

  const auto& values = json.at("values").get<picojson::array>();
  for (const auto& value : values) {
    if (value.get("start").is<double>()) {
      // 開始値と終了値を指定
      ease.add(value.at("type").get<std::string>(),
               value.at("start").get<double>(), value.at("end").get<double>(),
               value.at("duration").get<double>());
    }
    else {
      // 終了値のみの指定
      ease.add(value.at("type").get<std::string>(),
               value.at("end").get<double>(),
               value.at("duration").get<double>());
    }
  }

  return ease;
}


// MiniEasingを生成
template <typename T>
MiniEasing<T> miniEasingFromJson(const picojson::value& json) {
  return MiniEasing<T>(json.at("type").get<std::string>(),
                       json.at("duration").get<double>(),
                       vectFromJson<T>(json.at("start")),
                       vectFromJson<T>(json.at("end")));
}

template <>
MiniEasing<float> miniEasingFromJson(const picojson::value& json) {
  return MiniEasing<float>(json.at("type").get<std::string>(),
                           json.at("duration").get<double>(),
                           json.at("start").get<double>(),
                           json.at("end").get<double>());
}

template <typename T>
void miniEasingFromJson(MiniEasing<T>& easing, const picojson::value& json) {
  easing.start(json.at("type").get<std::string>(),
               json.at("duration").get<double>(),
               vectFromJson<T>(json.at("start")),
               vectFromJson<T>(json.at("end")));
}

template <>
void  miniEasingFromJson(MiniEasing<float>& easing, const picojson::value& json) {
  easing.start(json.at("type").get<std::string>(),
               json.at("duration").get<double>(),
               json.at("start").get<double>(),
               json.at("end").get<double>());
}

}
