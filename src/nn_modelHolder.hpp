
#pragma once

//
// モデルデータキャッシュ処理
//

#include "co_defines.hpp"
#include <string>
#include <unordered_map>
#include "co_matrix.hpp"
#include "co_model.hpp"


namespace ngs {

class ModelHolder {
  std::string path_;
	std::unordered_map<std::string, Model> models_;

  
public:
  explicit ModelHolder(const std::string& path) :
    path_(path)
  {
    DOUT << "ModelHolder()" << std::endl;
  }

  ~ModelHolder() {
    DOUT << "~ModelHolder()" << std::endl;
  }

  
  // MaterialやNodeの情報は書き換える事があるので実体を返す
  Model read(const std::string& name) {
		auto it = models_.find(name);
		if (it == models_.end()) {
      // まだ読み込んでないなら、読み込んでコンテナに格納する
			DOUT << "Model read: " << name << std::endl;
      // TIPS:引数が２つ以上あるクラスをemplaceする
      it = models_.emplace_hint(it, std::piecewise_construct,
                                    std::forward_as_tuple(name), std::forward_as_tuple(name, path_));
		}
    return it->second;
  }
};

}
