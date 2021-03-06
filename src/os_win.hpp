﻿
#pragma once

//
// OS依存処理(Windows版)
//

#if defined (_MSC_VER)

#include <windows.h>
#include <iostream>
#include <string>
#include <streambuf>
#include <vector>
#include <boost/noncopyable.hpp>


namespace ngs {

// デバッグコンソールにcoutする
class DbgStreambuf : public std::streambuf {
	std::vector<TCHAR> str_;
  // ↑OutputDebugString()がcharではなくTCHARを要求する

public:
	int_type overflow(int_type c = EOF) {
		str_.push_back(c);
		return c;
	}
	
	int sync() {
		// 念のため
		str_.push_back('\0');

		OutputDebugString(&str_[0]);
		str_.clear();
		return 0;
	}
};


class Os : private boost::noncopyable {
	DbgStreambuf dbgStream_;
	std::streambuf* stream_;

	std::string loadPath_;
	std::string savePath_;

public:
	Os() :
		loadPath_("res/"),
		savePath_("")
  {
    // エラーが発生したらメッセージボックスに表示する
		_set_error_mode(_OUT_TO_MSGBOX);
		// デバッグコンソールにcoutできるようにする
		stream_ = std::cout.rdbuf(&dbgStream_);

    DOUT << "Os()" << std::endl;
	}

	~Os() {
    DOUT << "~Os()" << std::endl;

    // coutの挙動を元に戻す
		std::cout.rdbuf(stream_);
	}

	const std::string& loadPath() const { return loadPath_; }
	const std::string& savePath() const { return savePath_; }
};

}

#endif
