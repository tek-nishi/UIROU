
#pragma once

//
// 雑多な処理
//

#include <iostream>
#include <string>
#include <regex>
#include <iomanip>


namespace ngs {

// floatの円周率
const float m_pi = static_cast<float>(M_PI);


// バイナリデータから値を生成
u_int getNum(const char* ptr, const u_int num) {
	u_int value = 0;

	for (u_int i = 0; i < num; ++i, ++ptr) {
		value = value + (static_cast<u_char>(*ptr) << (i * 8));
		// TIPS:int型より小さい型はint型に拡張されてから計算されるので8bit以上シフトしても問題ない
	}
#ifdef __BIG_ENDIAN__
	value = (value << 24) | ((value << 8) & 0xff0000) | ((value >> 8) & 0xff00) | (value >> 24);
#endif
			
	return value;
}


// ラジアン→度
template <typename T>
T rad2deg(const T rad) {
	return rad * 180 / m_pi;
}

// 度→ラジアン
template <typename T>
T deg2rad(const T deg) {
	return deg * m_pi / 180;
}


// 切り上げて一番近い２のべき乗値を求める
int int2pow(const int value) {
	int res = 1;

	while (res < (1 << 30)) {
		if (res >= value) break;
		res *= 2;
	}

	return res;
}

// 値を [min_value, max_value] にする
template <typename T>
T minmax(const T& value, const T& min_value, const T& max_value) {
  return std::max(std::min(value, max_value), min_value);
}


// 円周上の距離を角度(ラジアン)に変換
template <typename T>
T angleOnCircle(const T distance, const T radius) {
  return distance / radius;
}

// 角度(ラジアン)を円周上の距離に変換
template <typename T>
T distOnCircle(const T angle, const T radius) {
  return radius * angle;
}


// キーワード置換
void replaceString(std::string& text, const std::string& src, const std::string& dst) {
	std::string::size_type pos = 0;
	while ((pos = text.find(src, pos)) != std::string::npos) {
		text.replace(pos, src.length(), dst);
		pos += dst.length();
	}
}

// 文字列からCとC++のコメントを削除する
std::string removeComment(const std::string& text) {
  std::regex comment("(/\\*(?:.|\\n)*?\\*/)|(//[^\\n]*)");
  return std::regex_replace(text, comment, "");
}

// msをHH:MM:SSに変換する
std::string textFromTime(const float time_ms) {
  int time_s = int(time_ms);
  int second = time_s % 60;
  int minute = (time_s / 60) % 60;
  int hour   = (time_s / 60) / 60;

  std::ostringstream text;
  text << std::setw(2) << std::setfill('0') << hour << ":"
       << std::setw(2) << std::setfill('0') << minute << ":"
       << std::setw(2) << std::setfill('0') << second;
  
  return text.str();
}


// 被りにくい識別用の値を生成
// FIXME:More betterな実装
u_int createUniqueNumber() {
  static u_int unique_num = 0;
  return ++unique_num;
}


}
