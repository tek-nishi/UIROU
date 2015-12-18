
#pragma once

//
// カメラ
//

#include "co_defines.hpp"
#include "co_matrix.hpp"


namespace ngs {

class Camera {
	enum mode {
		FRUSTUM,
		PERSPECTIVE,
	};

	enum mode mode_;
	const Vec2f& size_;

	float fovy_;
	float near_;
	float far_;
	
	Mat4f proj_;
	Mat4f model_;

  float z_;
  
	Vec3f eye_;
  Quatf rotate_;

  
public:
  // 2D向け初期化
	Camera(const Vec2f& size, const float z) :
		mode_(FRUSTUM),
		size_(size),
    z_(z),
		proj_(Mat4f::Identity()),
    model_(Mat4f::Identity())
	{
		DOUT << "Camera(FRUSTUM)" << std::endl;
	}

  // 3D向け初期化
	Camera(const Vec2f& size, const float fovy, const float near_z, const float far_z) :
		mode_(PERSPECTIVE),
		size_(size),
		fovy_(fovy),
		near_(near_z),
		far_(far_z),
		proj_(Mat4f::Identity()),
    model_(Mat4f::Identity()),
    eye_(Vec3f::Zero()),
    rotate_(Quatf::Identity())
	{
		DOUT << "Camera(PERSPECTIVE)" << std::endl;
	}

	~Camera() {
		DOUT << "~Camera()" << std::endl;
	}

  
	Vec3f& eye() { return eye_; }
	const Vec3f& eye() const { return eye_; }

  Quatf& rotate() { return rotate_; }
  const Quatf& rotate() const { return rotate_; }

  float fovy() const { return fovy_; }
  void fovy(const float angle) { fovy_ = angle; }
  
  float farZ() const { return far_; }
  float nearZ() const { return near_; }
  
	const Mat4f& projection() const { return proj_; }
	const Mat4f& model() const { return model_; }
	

	void setup() {
		setMatrixMode(Matrix::PROJECTION);
		loadIdentity();
		switch (mode_) {
		case FRUSTUM:
			{
				float a = z_ * 2.0f;
				frustumMatrix(-size_.x() / a, size_.x() / a, -size_.y() / a, size_.y() / a, 1.0f, a);

				// z = 0 の時に正しい大きさで表示されるように設定
				setMatrixMode(Matrix::MODELVIEW);
        Eigen::Affine3f m;
        m = Eigen::Translation<float, 3>(0.0f, 0.0f, -z_);
        loadMatrix(m.matrix());
			}
			break;

		case PERSPECTIVE:
			{
				float aspect = size_.x() / size_.y();
				float fovy   = (aspect < 1.0f) ? verticalFovy() : fovy_;
				perspectiveMatrix(fovy, aspect, near_, far_);

				setMatrixMode(Matrix::MODELVIEW);
        
        Eigen::Affine3f m;
        m = Eigen::Translation<float, 3>(eye_) * rotate_;
        loadMatrix(m.matrix());
			}
			break;
		}
    
    proj_  = getProjectionMatrix();
    model_ = getModelMatrix();
	}

  
	Vec3f posToWorld(const Vec3f& pos, const Mat4f& model) const {
    GLint view[4];
		glGetIntegerv(GL_VIEWPORT, view);

    Mat4f model_camera = model_ * model;
    
    Vec3f res;
		pointUnProject(res, pos, model_camera, proj_, view);
		return res;
	}

	Vec3f posToScreen(const Vec3f& pos, const Mat4f& model) const {
    GLint view[4];
		glGetIntegerv(GL_VIEWPORT, view);

    Mat4f model_camera = model_ * model;
    
    Vec3f res;
		pointProject(res, pos, model_camera, proj_, view);
		return res;
	}


private:
  float horizonalFovy() {
    // fovyとnear_zから投影面の高さの半分を求める
    float half_h = std::tan(fovy_ / 2) * near_;

    // 表示画面の縦横比から、投影面の幅の半分を求める
    float aspect = size_.y() / size_.x();
    float half_w = half_h * aspect;

    // 投影面の幅の半分とnear_zから、横画面の時のfovyが求まる
    return std::atan(half_w / near_) * 2;
  }
  
  float verticalFovy() {
    // fovyとnear_zから投影面の幅の半分を求める
    float half_w = std::tan(fovy_ / 2) * near_;

    // 表示画面の縦横比から、投影面の高さの半分を求める
    float aspect = size_.y() / size_.x();
    float half_h = half_w * aspect;

    // 投影面の高さ半分とnear_zから、縦画面の時のfovyが求まる
    return std::atan(half_h / near_) * 2;
  }
	
};

}
