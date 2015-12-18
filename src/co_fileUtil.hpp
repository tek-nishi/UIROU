
#pragma once

//
// ファイル関連の雑用
//

#include <sys/stat.h>
#include <string>

namespace ngs {

// ファイルのパスだけ返す
std::string getFilePath(const std::string& path) {
	std::string::size_type end(path.rfind('/'));
	return (end != std::string::npos) ? path.substr(0, end) : std::string();
}

// パスを除いたファイル名を返す
std::string getFileNameFull(const std::string& path) {
	return path.substr(path.rfind('/') + 1, path.length());
}

// パスと拡張子を除いたファイル名を返す
std::string getFileName(const std::string& path) {
	std::string name = getFileNameFull(path);
	return name.substr(0, name.rfind('.'));
}

// 拡張子を除いたファイル名を返す
std::string getFileNameNoExt(const std::string& path) {
	std::string::size_type pos(path.rfind('.'));
	return path.substr(0, pos);
}

// 拡張子を返す
std::string getFileExt(const std::string& path) {
	std::string::size_type pos(path.rfind('.'));
	return (pos != std::string::npos) ? path.substr(pos, path.length()) : std::string();
}

// 拡張子を変更して返す
std::string changeFileExt(const std::string& path, const std::string& ext) {
	return getFileNameNoExt(path) + ext;
}


// ファイルの存在判定
bool isFileExists(const std::string& file)
{
	struct stat info;
	int result = stat(file.c_str(),&info);
	return (result == 0);
	// TODO: ディレクトリかどうかも判定
}

}
