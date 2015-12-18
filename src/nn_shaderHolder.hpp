
#pragma once

//
// シェーダーの簡易キャッシュ
//

#include "co_defines.hpp"
#include <string>
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include "co_easyShader.hpp"


namespace ngs {

class ShaderHolder : private boost::noncopyable {
  typedef std::shared_ptr<EasyShader> ShaderPtr;
  
  std::string path_;
	std::unordered_map<std::string, ShaderPtr> shaders_;

  
public:
  explicit ShaderHolder(const std::string& path) :
    path_(path)
  {
    DOUT << "ShaderHolder()" << std::endl;
  }

  ~ShaderHolder() {
    DOUT << "~ShaderHolder()" << std::endl;
  }

  
  std::shared_ptr<EasyShader> read(const std::string& name) {
		auto it = shaders_.find(name);
    if (it != shaders_.end()) {
      // 同じ名前のが見つかったらそれを返す
      return it->second;
    }

    // 新しく読み込んでキャッシュに登録
    DOUT << "Shader read: " << name << std::endl;
    ShaderPtr shader = std::make_shared<EasyShader>(path_ + name);

    // TIPS:shared_ptrなので、emplaceでなくて構わない
    shaders_.insert(std::unordered_map<std::string, ShaderPtr>::value_type(name, shader));
    
    return shader;
  }
  
  std::shared_ptr<EasyShader> get(const std::string& name) const {
		const auto it = shaders_.find(name);
    if (it != shaders_.cend()) {
      // 同じ名前のが見つかったらそれを返す
      return it->second;
    }

    // FIXME:見つからない場合に空のshared_ptrを返すのはよくない
    DOUT << "Shader not found: " << name << std::endl;
    return std::shared_ptr<EasyShader>();
  }
  
};

}
