
#pragma once

//
// OS依存処理(OSX版)
//

#if defined (__APPLE__) && !(TARGET_OS_IPHONE)

#include "co_defines.hpp"
#include <unistd.h>
#include <iostream>
#include <string>
#include <boost/noncopyable.hpp>
#include <CoreFoundation/CoreFoundation.h>


namespace ngs {


#ifdef _DEBUG

#ifndef SRCROOT
#define SRCROOT ./
#endif

// デバッグ時はプロジェクトのパスからファイルを読む
std::string currentPath() {
  return std::string(PREPRO_TO_STR(SRCROOT));
}

#else

// リリース時は.app内のファイルを読み込む
std::string currentPath() {
  return std::string("");
}

#endif

  
class Os : private boost::noncopyable {
	std::string load_path_;
	std::string save_path_;

public:
	Os() {
		DOUT << "Os()" << std::endl;

		std::string path = currentPath();
		load_path_ = path + "res/";
		save_path_ = path;
	}

	~Os() {
		DOUT << "~Os()" << std::endl;
	}

  
	const std::string& loadPath() const { return load_path_; }
	const std::string& savePath() const { return save_path_; }
};

}

#endif
