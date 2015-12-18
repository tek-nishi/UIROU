
#pragma once

//
// ベクトル定義
//

#include "co_defines.hpp"
#include <Eigen/Core>
#include <Eigen/Geometry>
#include "co_misc.hpp"


namespace ngs {

typedef Eigen::Matrix<GLint, 2, 1>	 Vec2i;
typedef Eigen::Matrix<GLfloat, 2, 1> Vec2f;
typedef Eigen::Matrix<GLfloat, 3, 1> Vec3f;
typedef Eigen::Matrix<GLfloat, 4, 1> Vec4f;
typedef Eigen::Quaternion<GLfloat>   Quatf;
typedef Eigen::Matrix<GLfloat, 4, 1> GrpCol;


// 反射(n: 法線 v:ベクトル r:反射係数 2.0で鏡面反射 1.0で反射なし)
Vec3f refrectVec(const Vec3f& n, const Vec3f& v, const float r = 2.0) {
	float d = n.dot(v) * r;
	Vec3f res = v - n * d;
	return res;
}

// 2つのベクトルの角度を求める[0,  m_pi]
// Vec2fの場合は外積を使えば [-m_pi, m_pi]にできる
template <typename T>
float angleFromVecs(const T& v1, const T& v2) {
  float l1 = v1.norm();
  float l2 = v2.norm();
  if ((l1 <= 0.0f) || (l2 <= 0.0f)) {
    // どちらかのベクトルの長さがゼロの場合、角度は０
    return 0.0f;
  }

  // acosに渡す値は[-1, 1]
  float a = minmax(v1.dot(v2) / (l1 * l2), -1.0f, 1.0f);
  return std::acos(a);
}

// 向きがランダムな単位ベクトルを生成
template <typename T>
T randomVector() {
  T v = T::Random();
  if (!v.squaredNorm()) {
    // 長さが０の場合はx成分を1にしておく
    v.x() = 1.0f;
  }
  return v.normalized();
}

// 指定した長さのベクトルを生成
template <typename T>
T lengthVector(const T& vec, const float length) {
  if (!vec.squaredNorm()) return vec;
  return vec.normalized() * length;
}

// 入力と垂直な任意の単位ベクトルを求める
Vec3f verticalVector(const Vec3f& vec) {
  Vec3f res(Vec3f::Random());
  if (!res.squaredNorm()) {
    // 長さが０の場合はx成分を1にしておく
    res.x() = 1.0f;
  }
  
  // 入力ベクトルとランダムなベクトルとの外積を解けば、垂直なベクトルが求まる
  if (vec.x() != 0.0f) {
    res.x() = (-vec.y() * res.y() - vec.z() * res.z()) / vec.x();
  }
  else if (vec.y() != 0.0f) {
    res.y() = (-vec.x() * res.x() - vec.z() * res.z()) / vec.y();
  }
  else {
    res.z() = (-vec.x() * res.x() - vec.y() * res.y()) / vec.z();
  }
  
  return res.normalized();
}

// startとendをt[0.0, 1.0] で球面線形補間したベクトルを求める
Vec3f slerpVector(const Vec3f& start, const Vec3f& end, const float t) {
  float angle = angleFromVecs(start, end);
  float sinTh = std::sin(angle);
  float ps = std::sin(angle * (1 - t));
  float pe = std::sin(angle * t);
  return (start * ps + end * pe) / sinTh;
}

}
