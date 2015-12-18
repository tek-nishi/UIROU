
#pragma once

//
// 簡単シェーダー
//

#include "co_defines.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>
#include <boost/noncopyable.hpp>
#include "co_misc.hpp"


namespace ngs {

// TIPS:複数行の文字列を単一の文字列にラップする
#define SHADER_PROG(A) #A


class EasyShader : private boost::noncopyable {
	GLuint program_;
	std::unordered_map<std::string, GLint> attribs_;
	std::unordered_map<std::string, GLint> uniforms_;

  
	GLuint compile(GLuint type, const std::string& source) {
    // コメント除去
    std::string text = removeComment(source);

		// シェーダーの中のattributeとuniform変数をリストアップ
		listupTokens(attribs_, text, "attribute");
		listupTokens(uniforms_, text, "uniform");

    // TODO:以下のコードはC++11のregexで書き換えられる
#if (TARGET_OS_IPHONE)
		// ES 2.0:floatの精度を指定
		replaceString(text, "es_lowp;", "precision lowp float;");
		replaceString(text, "es_mediump;", "precision mediump float;");
		replaceString(text, "es_highp;", "precision highp float;");
#else
		// ES 2.0以外:要らない文字列を削除
		replaceString(text, "es_lowp;", "");
		replaceString(text, "es_mediump;", "");
		replaceString(text, "es_highp;", "");

		replaceString(text, "highp", "");
		replaceString(text, "mediump", "");
		replaceString(text, "lowp", "");
#endif

		GLuint shader = glCreateShader(type);
    // glShaderSource は char** を要求する
    const char* text_ptr = text.c_str();
		glShaderSource(shader, 1, &text_ptr, 0);
		glCompileShader(shader);

		GLint compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
#ifdef _DEBUG
		if (compiled == GL_FALSE) {
			GLint length;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
			std::string log(length + 1, ' ');
			glGetShaderInfoLog(shader, length, &length, &log[0]);
			DOUT << "compile error:" << log << std::endl;

			return GL_FALSE;
		}
#endif
		return shader;
	}

	void setup(const std::string& v_source, const std::string& f_source) {
		GLuint vertex_shader = compile(GL_VERTEX_SHADER, v_source);
		GLuint fragment_shader = compile(GL_FRAGMENT_SHADER, f_source);

		glAttachShader(program_, vertex_shader);
		glAttachShader(program_, fragment_shader);
		glLinkProgram(program_);
#ifdef _DEBUG
		GLint status;
    glGetProgramiv(program_, GL_LINK_STATUS, &status);
		if (status == GL_FALSE) {
			GLint length;
			glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &length);
			std::string log(length + 1, ' ');
			glGetProgramInfoLog(program_, length, &length, &log[0]);
			DOUT << "link error:" << log << std::endl;
		}
#endif
		
		// リストアップしたattributeとuniform変数をまとめて関連づけ
		// FIXME:mapをforでまわすのはいくない
    for (auto& it : attribs_) {
			it.second = glGetAttribLocation(program_, it.first.c_str());
			DOUT << "Attrib:" << it.first << " " << it.second << std::endl;
			assert(it.second >= 0);
    }
		
		for (auto& it : uniforms_) {
			it.second = glGetUniformLocation(program_, it.first.c_str());
			DOUT << "Uniform:" << it.first << " " << it.second << std::endl;
			assert(it.second >= 0);
		}

		// リンクしたらもう要らない
		if (vertex_shader) {
			glDetachShader(program_, vertex_shader);
			glDeleteShader(vertex_shader);
		}

		if (fragment_shader) {
			glDetachShader(program_, fragment_shader);
			glDeleteShader(fragment_shader);
		}
	}

public:
	EasyShader(const std::string& v_source, const std::string& f_source) :
		program_(glCreateProgram())
	{
		DOUT << "EasyShader()" << std::endl;
		setup(v_source, f_source);
	}

	explicit EasyShader(const std::string& file) :
		program_(glCreateProgram())
	{
		DOUT << "EasyShader()" << std::endl;

    // .vshを読み込む
    std::string vsh_path = file + ".vsh";
		std::ifstream vsh_fs(vsh_path);
		assert(vsh_fs);
		std::string v_source((std::istreambuf_iterator<char>(vsh_fs)), std::istreambuf_iterator<char>());

    // .fshを読み込む
    std::string fsh_path = file + ".fsh";
		std::ifstream fsh_fs(fsh_path);
		assert(fsh_fs);
		std::string f_source((std::istreambuf_iterator<char>(fsh_fs)), std::istreambuf_iterator<char>());
		
		setup(v_source, f_source);
	}

	~EasyShader() {
    glDeleteProgram(program_);
	}

  
	void operator()() const { glUseProgram(program_); }

	GLint attrib(const std::string& name) const {
		const auto it = attribs_.find(name);
		assert(it != attribs_.cend());
		return it->second;
	}
	
	GLint uniform(const std::string& name) const {
		const auto it = uniforms_.find(name);
		assert(it != uniforms_.cend());
		return it->second;
	}

  // シェーダーの正当性をチェック
	bool validate() {
		glValidateProgram(program_);

		GLint length;
		glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &length);
		std::string log(length + 1, ' ');
		glGetProgramInfoLog(program_, length, &length, &log[0]);
		DOUT << "validate log:" << log << std::endl;

		GLint status;
		glGetProgramiv(program_, GL_VALIDATE_STATUS, &status);
		return status == GL_TRUE;
	}


  // 識別子をリストアップ
  static void listupTokens(std::unordered_map<std::string, GLint>& tokens,
                           const std::string& text, const std::string& keyword) {
    std::string::size_type pos = 0;
    while ((pos = text.find(keyword, pos)) != std::string::npos) {
      // キーワードを見つけたら、行末の";"を探す
      std::string::size_type epos = text.find(";", pos);
		
      // 識別子をゲット
      std::string::size_type n_pos = text.rfind(" ", epos);
      std::string s = text.substr(n_pos + 1, epos - (n_pos + 1));

      // TIPS:hoge[3] という書式にも対応
      std::string::size_type cpos  = s.rfind("[");
      if (cpos != std::string::npos) s = s.substr(0, cpos);

      // これまでに登録されているキーワードと被っていなければ追加
      if (tokens.find(s) == tokens.end()) {
        tokens.insert(std::unordered_map<std::string, GLint>::value_type(s, 0));
      }

      pos = epos + 1;
    }
  }
};

}
