
#pragma once

//
// 衝突判定
// SOURCE:Real-Time Collision Detection by Christer Ericson
//

#include <cfloat>
#include "co_matrix.hpp"


namespace ngs {


// 判定に使う直方体(axis-aligned bounding box)
struct AABBVolume {
	// 中心位置
	Vec3f point;
	// 中心からの距離
	Vec3f radius;
};

// 判定に使う球
struct SphereVolume {
	// 中心位置
	Vec3f point;
	// 半径
	float radius;
};


// 球aと球bの接触判定
bool testSpheres(const SphereVolume& a, const SphereVolume& b) {
	// aとbの中心点の距離の平方を求める
	Vec3f d = a.point - b.point;
	float square = d.dot(d);

	float r = a.radius + b.radius;
	return square <= (r * r);
}

// 点pとAABBとの距離の平方
float squarePointAABB(const Vec3f& p, const AABBVolume& b) {
	float square = 0.0f;
	for (u_int i = 0; i < 3; ++i) {
		float p_min = b.point(i) - b.radius(i);
		float p_max = b.point(i) + b.radius(i);
		float v = p(i);
		if (v < p_min) square += (p_min - v) * (p_min - v);
		if (v > p_max) square += (v - p_max) * (v - p_max);
	}
	return square;
}

// 点pとAABBとの最接近点
void closestPointAABB(Vec3f& res, const Vec3f& p, const AABBVolume& b) {
	for (u_int i = 0; i < 3; ++i) {
		float p_min = b.point(i) - b.radius(i);
		float p_max = b.point(i) + b.radius(i);
		float v = p(i);
		if (v < p_min) v = p_min;
		if (v > p_max) v = p_max;
		res(i) = v;
	}
}

// 球aとAABBの接触判定
bool testSphereAABB(const SphereVolume& a, const AABBVolume& b) {
	float square = squarePointAABB(a.point, b);
	return square <= (a.radius * a.radius);
}

// 球aとAABBの接触判定 & 最接近点
bool testSphereAABB(Vec3f& res, const SphereVolume& a, const AABBVolume& b) {
	closestPointAABB(res, a.point, b);
	Vec3f v = res - a.point;
	return v.dot(v) <= (a.radius * a.radius);
}

// 球と光線(p + td)との交差判定
bool testRaySphere(const Vec3f& p, const Vec3f& d, const SphereVolume& s) {
	Vec3f m = p - s.point;

	float c = m.dot(m) - s.radius * s.radius;
	if (c <= 0.0f) return true;

	float b = m.dot(d);
	if (b > 0.0f) return false;

	float disc = b * b - c;
	if (disc < 0.0f) return false;

	return true;
}

// 球と光線(p + td)との交差判定と交差点
bool testRaySphere(Vec3f& res, float& res_t, const Vec3f& p, const Vec3f& d, const SphereVolume& s) {
	Vec3f m = p - s.point;

	float b = m.dot(d);
	float c = m.dot(m) - s.radius * s.radius;
	if ((c > 0.0f) && (b > 0.0f)) return false;

	float disc = b * b - c;
	if (disc < 0.0f) return false;

	float t = -b - std::sqrt(disc);
	if (t < 0.0f) t = 0.0f;

  res_t = t;
	res = p + t * d;

	return true;
}

// 球と線分(start, end)との交差判定
bool testLineSphere(Vec3f& res, float& res_t, const SphereVolume& volume, const Vec3f& pos_start, const Vec3f& pos_end) {
  return testRaySphere(res, res_t,
                       pos_start, Vec3f(pos_end - pos_start).normalized(),
                       volume);
}

// 点p0→p1とAABBとの交差判定
bool testSegmentAABB(const Vec3f& p0, const Vec3f& p1, const AABBVolume& b) {
	Vec3f b_max = b.point + b.radius;
	Vec3f e = b_max - b.point;
	Vec3f m = (p0 + p1) * 0.5f;
	Vec3f d = p1 - m;
	m = m - b.point;

	float adx = std::abs(d.x());
	if (std::abs(m.x()) > e.x() + adx) return false;
	float ady = std::abs(d.y());
	if (std::abs(m.y()) > e.y() + ady) return false;
	float adz = std::abs(d.z());
	if (std::abs(m.z()) > e.z() + adz) return false;

	adx += FLT_EPSILON;
	ady += FLT_EPSILON;
	adz += FLT_EPSILON;

	if (std::abs(m.y() * d.z() - m.z() * d.y()) > e.y() * adz + e.z() * ady) return false;
	if (std::abs(m.z() * d.x() - m.x() * d.z()) > e.x() * adz + e.z() * adx) return false;
	if (std::abs(m.x() * d.y() - m.y() * d.x()) > e.x() * ady + e.y() * adx) return false;

	return true;
}

// AABBと光線(p + td)との交差判定と交差点
bool testRayAABB(Vec3f& res, const Vec3f& p, const Vec3f& d, const AABBVolume& b) {
	float tmin = 0.0;
	float tmax = FLT_MAX;

	for (u_int i = 0; i < 3; ++i) {
		float b_min = b.point(i) - b.radius(i);
		float b_max = b.point(i) + b.radius(i);

		if (std::abs(d(i)) < FLT_EPSILON) {
			if (p(i) < b_min || p(i) > b_max) return false;
		}
		else {
			float ood = 1.0f / d(i);
			float t1 = (b_min - p(i)) * ood;
			float t2 = (b_max - p(i)) * ood;

			if (t1 > t2) std::swap(t1, t2);
			if (t1 > tmin) tmin = t1;
			if (t2 > tmax) tmax = t2;
			if (tmin > tmax) return false;
		}
	}
	res = p + d * tmin;

	return true;
}


struct Plane {
	Vec3f n;
	float d;
};

// 三点(時計回り)から平面の方程式を求める
Plane computePlane(const Vec3f& a, const Vec3f& b, const Vec3f& c) {
	Plane p;

	p.n = ((b - a).cross(c - a)).normalized();
	p.d = p.n.dot(a);
	
	return p;
}

// 点qと平面pとの距離を求める
// マイナスの場合、点は平面の下側にある
float distPointPlane(const Vec3f& q, const Plane& p) {
	return (p.n.dot(q) - p.d) / p.n.dot(p.n);
}

// 点qがAABBの上方向にある面をリストアップ
void planesPointAABB(std::vector<Plane>& res, const Vec3f& q, const AABBVolume& b) {
	Plane p;

	for (u_int i = 0; i < 3; ++i) {
		const float value[] = { -1.0f, 1.0f };
		for (u_int h = 0; h < 2; ++h) {
			p.n = Vec3f::Zero();
			p.n(i) = value[h];
			p.d = b.radius(i);
			if (distPointPlane(q, p) >= 0.0f) {
				res.push_back(p);
			}
		}
	}
}


struct Box {
  // 左上座標
  Vec2f inf;
  // 右下座標
  Vec2f sup;
};

// 点と矩形
bool testPointBox(const Vec2f& pos, const Box& box) {
  return pos.x() >= box.inf.x()
      && pos.x() <= box.sup.x()
      && pos.y() >= box.inf.y()
      && pos.y() <= box.sup.y();
}



}
