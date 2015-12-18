
#pragma once

//
// セーブデータ
//

#include "co_json.hpp"
#include "co_framework.hpp"


namespace ngs {

class Settings {
  // 記録ファイル名(フルパス)
  std::string save_path_;
  // 記録
  picojson::value value_;
  

public:
  Settings(const Framework& fw, const std::string& file_name) :
    save_path_(fw.savePath() + file_name),
    value_(read(save_path_, fw.loadPath() + file_name))
  {
    // バージョンアップする必要があるので、オリジナルを読む
    picojson::value settings = readJsonFile(fw.loadPath() + file_name);

    double current_ver = settings.at("version").get<double>();
    double saved_ver = value_.at("version").get<double>();
    if (saved_ver < current_ver) {
      // 記録の更新
      DOUT << "Update settings " << saved_ver << " to " << current_ver << std::endl;
      updateSettings(value_, settings);

      // 更新したらすぐに書き出す
      write();
    }
  }

  
  picojson::value& value() { return value_; }
  const picojson::value& value() const { return value_; }
  

  // 記録を書き出す
  void write() const {
    // writeJson(value_, save_path_ + ".json");
    writeJsonZlib(value_, save_path_ + ".save");
  }


private:
  // 記録を読み込む
  static picojson::value read(const std::string& save_path, const std::string& load_path) {
    std::string packed_file(save_path + ".save");
    if (isFileExists(packed_file)) {
      // セーブデータがあれば
      DOUT << "Settings read from " << packed_file << std::endl;
      return readJsonZlib(packed_file);
    }
    else {
      // 無ければアプリ内のデータを読む
      return readJsonFile(load_path);
    }
  }

  // 圧縮したのを優先してjsonを読み込む
  static picojson::value readJsonFile(const std::string& file) {
    std::string packed_file(file + ".pack");
    if (isFileExists(packed_file)) {
      // 圧縮されたのがあれば
      DOUT << "Settings read from " << packed_file << std::endl;
      return readJsonZlib(packed_file);
    }
    else {
      // 通常
      DOUT << "Settings read from " << file << ".json" << std::endl;
      return readJson(file + ".json");
    }
  }

  // 記録を新しいバージョンに更新
  static void updateSettings(picojson::value& value, const picojson::value& new_value) {
    // バージョン番号を更新
    value.at("version") = new_value.at("version");

    // 新しい要素を追加
    const picojson::object& new_settings = new_value.get<picojson::object>();
    for (auto it = new_settings.cbegin(); it != new_settings.end(); ++it) {
      if (!value.contains(it->first)) {
        value.get<picojson::object>().insert(picojson::object::value_type(it->first, it->second));
      }
    }

    // 「評価する」ダイアログを再表示(iOS)
    rating::reset();
    advertisement::reset();
  }
  
};

}
