
#pragma once

// 
// 行列定義
//

#include "co_defines.hpp"
#include <deque>
#include <cassert>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include "co_vector.hpp"


namespace ngs {

typedef Eigen::Matrix4f							 Mat4f;
typedef Eigen::Matrix3f							 Mat3f;
typedef Eigen::Quaternion<GLfloat>   Quatf;


class Matrix {
public:
	enum Mode {
		PROJECTION,
		MODELVIEW,
	};

private:
	std::deque<Mat4f> projection_stack_;
	std::deque<Mat4f> model_stack_;
	std::deque<Mat4f>* current_;

	Mode mode_;
	
public:
	Matrix() :
		current_(&projection_stack_),
		mode_(PROJECTION)
	{
		projection_stack_.push_back(Mat4f::Identity());
		model_stack_.push_back(Mat4f::Identity());
	}

	void mode(const Mode mode) {
		mode_ = mode;
		switch (mode)
		{
		case PROJECTION:
			current_ = &projection_stack_;
      return;

		case MODELVIEW:
			current_ = &model_stack_;
      return;
		
		default:
			assert(0);
		}
	}

	Mat4f& current() { return current_->back(); }
	Mat4f& projection() { return projection_stack_.back(); }
	Mat4f& model() { return model_stack_.back(); }

	void push() { current_->push_back(current()); }
	void pop() { current_->pop_back(); }
};

Matrix matrix_stack;


Mat4f& getCurrentMatrix() {
	return matrix_stack.current();
}

Mat4f& getProjectionMatrix() {
	return matrix_stack.projection();
}

Mat4f& getModelMatrix() {
	return matrix_stack.model();
}

// モデルマトリックス→3x3
Mat3f getNormalMatrix() {
	Mat4f& current_matrix = getModelMatrix();
	// 4x4 to 3x3
	Mat3f normal_matrix = current_matrix.block(0,0,3,3);

	return normal_matrix;
}

void setMatrixMode(const Matrix::Mode mode) {
	matrix_stack.mode(mode);
}

void loadIdentity() {
	matrix_stack.current() = Mat4f::Identity();
}

void pushMatrix() {
	matrix_stack.push();
}

void popMatrix() {
	matrix_stack.pop();
}


void rotateMatrix(const GLfloat angle, const Vec3f& vec) {
	Eigen::Affine3f m;
	m = Quatf(Eigen::AngleAxisf(angle, vec));
	
	Mat4f& cur = getCurrentMatrix();
	cur *= m.matrix();
}

void scaleMatrix(const Vec3f& scale) {
	Eigen::Affine3f m;
	m = Eigen::Scaling(scale);

	Mat4f& cur = getCurrentMatrix();
	cur *= m.matrix();
}

void translateMatrix(const Vec3f& translate) {
	Eigen::Affine3f m;
	m = Eigen::Translation<float, 3>(translate);
	
	Mat4f& cur = getCurrentMatrix();
	cur *= m.matrix();
}

void loadMatrix(const Mat4f& mt) {
	Mat4f& cur = getCurrentMatrix();
	cur = mt;
}

void multMatrix(const Mat4f& mt) {
	Mat4f& cur = getCurrentMatrix();
	cur *= mt;
}
	

// 正投影行列を生成してカレントの行列に掛け合わせる
// SOURCE:mesa
void orthoMatrix(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat nearval, GLfloat farval) {
	Mat4f m;

	m(0,0) = 2.0 / (right-left);
	m(0,1) = 0.0;
	m(0,2) = 0.0;
	m(0,3) = -(right+left) / (right-left);

	m(1,0) = 0.0;
	m(1,1) = 2.0 / (top-bottom);
	m(1,2) = 0.0;
	m(1,3) = -(top+bottom) / (top-bottom);

	m(2,0) = 0.0;
	m(2,1) = 0.0;
	m(2,2) = -2.0 / (farval-nearval);
	m(2,3) = -(farval+nearval) / (farval-nearval);

	m(3,0) = 0.0;
	m(3,1) = 0.0;
	m(3,2) = 0.0;
	m(3,3) = 1.0;

  multMatrix(m);
	// Mat4f& cur = getCurrentMatrix();
	// cur *= m;
}

// 透視投影行列を生成してカレントの行列に掛け合わせる
// SOURCE:mesa
void frustumMatrix(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat nearval, GLfloat farval) {
	GLfloat x, y, a, b, c, d;

	x = (2.0*nearval) / (right-left);
	y = (2.0*nearval) / (top-bottom);
	a = (right+left) / (right-left);
	b = (top+bottom) / (top-bottom);
	c = -(farval+nearval) / ( farval-nearval);
	d = -(2.0*farval*nearval) / (farval-nearval);  /* error? */

	Mat4f m;
	m(0,0) = x;
	m(0,1) = 0.0;
	m(0,2) = a;
	m(0,3) = 0.0;
	 
	m(1,0) = 0.0;
	m(1,1) = y;
	m(1,2) = b;
	m(1,3) = 0.0;
	 
	m(2,0) = 0.0;
	m(2,1) = 0.0;
	m(2,2) = c;
	m(2,3) = d;
	 
	m(3,0) = 0.0;
	m(3,1) = 0.0;
	m(3,2) = -1.0;
	m(3,3) = 0.0;

  multMatrix(m);
	// Mat4f& cur = getCurrentMatrix();
	// cur *= m;
}

// 透視投影行列を生成してカレントの行列に掛け合わせる
// SOURCE:mesa
void perspectiveMatrix(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar) {
	GLfloat sine, cotangent, deltaZ;
	GLfloat radians = fovy / 2.0;

	deltaZ = zFar - zNear;
	sine = std::sin(radians);
	assert((deltaZ != 0.0) && (sine != 0.0) && (aspect != 0.0));
	cotangent = std::cos(radians) / sine;

	Mat4f m;
	m = Mat4f::Identity();
	m(0,0) = cotangent / aspect;
	m(1,1) = cotangent;
	m(2,2) = -(zFar + zNear) / deltaZ;
	m(2,3) = -2.0 * zNear * zFar / deltaZ;
	m(3,2) = -1.0;
	m(3,3) = 0.0;

	Mat4f& cur = getCurrentMatrix();
	cur *= m;
}

// 視点移動行列を生成してカレントの行列に掛け合わせる
// SOURCE:mesa
void lookAtMatrix(const Vec3f& eye, const Vec3f& center, const Vec3f& up) {
	Vec3f forward;
	Vec3f side;
	Vec3f upv;

	forward = center - eye;
	forward.normalize();
	upv = up;

	side = forward.cross(upv);
	side.normalize();
	upv = side.cross(forward);

	Mat4f m;
	m = Mat4f::Identity();

	m(0,0) = side.x();
	m(0,1) = side.y();
	m(0,2) = side.z();

	m(1,0) = upv.x();
	m(1,1) = upv.y();
	m(1,2) = upv.z();

	m(2,0) = -forward.x();
	m(2,1) = -forward.y();
	m(2,2) = -forward.z();

  multMatrix(m);
	// Mat4f& cur = getCurrentMatrix();
	// cur *= m;

	translateMatrix(-eye);
}


// 3D座標→投影座標
// SOURCE:mesa
bool pointProject(Vec3f& win, const Vec3f& obj, const Mat4f& modelMatrix, const Mat4f& projMatrix, const int viewport[4]) {
	Vec4f in(obj.x(), obj.y(), obj.z(), 1.0);
	Vec4f out = modelMatrix * in;
	in = projMatrix * out;
	if (in(3) == 0.0) return false;

	/* Map x, y and z to range 0-1 */
	for (u_int i = 0; i < 3; ++i) {
		float a = in(i) / in(3);
		in(i) = a * 0.5 + 0.5;
	}

	/* Map x,y to viewport */
	in(0) = in(0) * viewport[2] + viewport[0];
	in(1) = in(1) * viewport[3] + viewport[1];

	win << in.x(), in.y(), in.z();

	return true;
}

// 投影座標→3D座標
// SOURCE:mesa
bool pointUnProject(Vec3f& obj, const Vec3f& win, const Mat4f& modelMatrix, const Mat4f& projMatrix, const int viewport[4]) {
	Vec4f in(win.x(), win.y(), win.z(), 1.0);

	/* Map x and y from window coordinates */
	in(0) = (in(0) - viewport[0]) / viewport[2];
	in(1) = (in(1) - viewport[1]) / viewport[3];

	/* Map to range -1 to 1 */
	for (u_int i = 0; i < 3; ++i) {
		in(i) = in(i) * 2.0 - 1.0;
	}

	// 座標に逆行列を掛けて完成
	Mat4f finalMatrix =  projMatrix * modelMatrix;
	Vec4f out = finalMatrix.inverse() * in;
	if (out(3) == 0.0) return false;

	for (u_int i = 0; i < 3; ++i) {
		obj(i) = out(i) / out(3);
	}

	return true;
}


}
