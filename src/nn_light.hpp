
#pragma once

//
// 光源
//

#include "co_matrix.hpp"
#include "co_easyShader.hpp"


namespace ngs {

class Light {
  // 遠くにある点光源→平行光源 なので、向きは持たなくてよい
  Vec3f pos_;

	Vec3f ambient_;
	Vec3f diffuse_;
	Vec3f specular_;

public:
  Light() :
    pos_(Vec3f::Zero()),
    ambient_(0.5, 0.5, 0.5),
    diffuse_(Vec3f::Ones()),
    specular_(Vec3f::Ones())
  {
    DOUT << "Light" << std::endl;
  }

  ~Light() {
    DOUT << "~Light" << std::endl;
  }

  
  Vec3f& pos() { return pos_; }
  const Vec3f& pos() const { return pos_; }

	Vec3f& ambient() { return  ambient_; }
	const Vec3f& ambient() const { return  ambient_; }

	Vec3f& diffuse() { return diffuse_; }
	const Vec3f& diffuse() const { return diffuse_; }

  Vec3f& specular() { return specular_; }
  const Vec3f specular() const { return specular_; }
  

  // 光源をシェーダーにセット
  static void setup(const EasyShader& shader, const Light& light) {
    const Vec3f& pos      = light.pos();
    const Vec3f& ambient  = light.ambient();
    const Vec3f& diffuse  = light.diffuse();
    const Vec3f& specular = light.specular();
    
    shader();
    
		glUniform3f(shader.uniform("lightPosition"), pos.x(), pos.y(), pos.z());
		glUniform3f(shader.uniform("ambient"), ambient.x(), ambient.y(), ambient.z());
		glUniform3f(shader.uniform("diffuse"), diffuse.x(), diffuse.y(), diffuse.z());
		glUniform3f(shader.uniform("specular"), specular.x(), specular.y(), specular.z());
  }
  
  static void setup(const EasyShader& shader, const Light& light_1, const Light& light_2, const Light& light_3) {
    const Vec3f& pos_1      = light_1.pos();
    const Vec3f& ambient_1  = light_1.ambient();
    const Vec3f& diffuse_1  = light_1.diffuse();
    const Vec3f& specular_1 = light_1.specular();

    const Vec3f& pos_2      = light_2.pos();
    const Vec3f& ambient_2  = light_2.ambient();
    const Vec3f& diffuse_2  = light_2.diffuse();
    const Vec3f& specular_2 = light_2.specular();

    const Vec3f& pos_3      = light_3.pos();
    // const Vec3f& ambient_3  = light_3.ambient();
    const Vec3f& diffuse_3  = light_3.diffuse();
    // const Vec3f& specular_3 = light_3.specular();
    
    shader();

    GLfloat pos[] = {
      pos_1.x(), pos_1.y(), pos_1.z(),
      pos_2.x(), pos_2.y(), pos_2.z(),
      pos_3.x(), pos_3.y(), pos_3.z()
    };
    GLfloat ambient[] = {
      ambient_1.x(), ambient_1.y(), ambient_1.z(),
      ambient_2.x(), ambient_2.y(), ambient_2.z()
      // ambient_3.x(), ambient_3.y(), ambient_3.z()
    };
    GLfloat diffuse[] = {
      diffuse_1.x(), diffuse_1.y(), diffuse_1.z(),
      diffuse_2.x(), diffuse_2.y(), diffuse_2.z(),
      diffuse_3.x(), diffuse_3.y(), diffuse_3.z()
    };
    GLfloat specular[] = {
      specular_1.x(), specular_1.y(), specular_1.z(),
      specular_2.x(), specular_2.y(), specular_2.z()
      // specular_3.x(), specular_3.y(), specular_3.z()
    };
    
		glUniform3fv(shader.uniform("lightPosition"), 3, pos);
		glUniform3fv(shader.uniform("ambient"),       2, ambient);
		glUniform3fv(shader.uniform("diffuse"),       3, diffuse);
		glUniform3fv(shader.uniform("specular"),      2, specular);
  }

  
};


}
