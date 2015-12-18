
#pragma once

//
// 描画環境
//

#include "co_defines.hpp"
#include "co_vector.hpp"


namespace ngs {

class View {
	// 実ウインドウサイズ
	GLfloat width_;
	GLfloat height_;

	// 表示スケーリング(Retina対応用)
	GLfloat scale_;

	// アプリ内での描画サイズ
	Vec2f size_;

  GrpCol clear_color_;

  // 上部オフセット(iAd対応用)
  float top_offset_;
  
  void setViewSize() {
		size_ << width_ * scale_,
             height_ * scale_;
  }

  
public:
	View() :
    width_(640.0f),
    height_(480.0f),
    scale_(1.0f),
    size_(width_, height_),
    clear_color_(GrpCol::Zero()),
    top_offset_(0.0f)
  {
		DOUT << "View()" << std::endl;
	}
	
	~View() {
		DOUT << "~View()" << std::endl;
	}

  
  void setup(const int width, const int height, const float scale) {
    width_  = width;
    height_ = height;
    scale_  = scale;
    setViewSize();
  }

	void resize(const int width, const int height) {
		width_  = width;
		height_ = height;

    setViewSize();
    setupViewport();
	}

  // マウス座標をスクリーン座標(画面の中心が(0,0)の座標系)に変換
	Vec2f toScreenPos(const float x, const float y) const {
    return Vec2f((x - width_ / 2.0f) * scale_,
                 (y - height_ / 2.0f) * -scale_);
	}

  // スクリーン座標をWindow座標(左上が(0, height_)の座標系)に変換
  Vec2f toWindowPos(const Vec2f& pos) const {
    return Vec2f(pos.x() / scale_ + width_ / 2.0f,
                 pos.y() / scale_ + height_ / 2.0f);
  }

  const Vec2f& size() const { return size_; }
  float scale() const { return scale_; }

  void clearColor(const GrpCol& color) { clear_color_ = color; }

  // 画面上部のオフセットを指定
  void topOffset(const float offset) {
    top_offset_ = offset * scale_;
  }

  
  void setupDraw() const {
    // FIXME:いい実装を思いつくまではハードコーディング
		// glClearColor(clear_color_(0), clear_color_(1), clear_color_(2), clear_color_(3));
		// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		// glEnable(GL_DEPTH_TEST);
		// glEnable(GL_CULL_FACE);
		// glCullFace(GL_BACK);
  }

  void setupViewport() const {
    glViewport(0, 0, width_, height_);
  }


  enum Layout {
    CENTER = 0,
    TOP    = (1 << 0),
    BOTTOM = (1 << 1),
    LEFT   = (1 << 2),
    RIGHT  = (1 << 3)
  };

  // 画面四隅を考慮した座標を計算(iOSの縦横画面に対応させられる)
  Vec2f layoutPos(const Vec2f& pos, Layout type = CENTER) const {
    Vec2f layouted_pos = pos;
    float w            = width_ / 2.0f * scale_;
    float h            = height_ / 2.0f * scale_;

    if (type & TOP)    layouted_pos.y() += h - top_offset_;
    if (type & BOTTOM) layouted_pos.y() -= h;
    if (type & LEFT)   layouted_pos.x() -= w;
    if (type & RIGHT)  layouted_pos.x() += w;

    if (!(type & (TOP | BOTTOM))) layouted_pos.y() -= top_offset_ / 2.0f;
    
    return layouted_pos;
  }
  
  // 文字列からLayoutを決める
  static Layout layoutFromString(const std::string& text) {
    static const std::pair<std::string, Layout> tbl[] = {
      std::make_pair("center", Layout::CENTER),

      std::make_pair("top",    Layout::TOP),
      std::make_pair("bottom", Layout::BOTTOM),
      std::make_pair("left",   Layout::LEFT),
      std::make_pair("right",  Layout::RIGHT),

      // FIXME:下のoperatorが適用出来ないので泣く泣くキャスト
      std::make_pair("top_left",  Layout(Layout::TOP | Layout::LEFT)),
      std::make_pair("top_right", Layout(Layout::TOP | Layout::RIGHT)),
      // top_leftでもleft_topでも通じるように
      std::make_pair("left_top",  Layout(Layout::TOP | Layout::LEFT)),
      std::make_pair("right_top", Layout(Layout::TOP | Layout::RIGHT)),

      std::make_pair("bottom_left",  Layout(Layout::BOTTOM | Layout::LEFT)),
      std::make_pair("bottom_right", Layout(Layout::BOTTOM | Layout::RIGHT)),
      // bottom_leftでもleft_bottomでも通じるように
      std::make_pair("left_bottom",  Layout(Layout::BOTTOM | Layout::LEFT)),
      std::make_pair("right_bottom", Layout(Layout::BOTTOM | Layout::RIGHT))
    };

    for (const auto& value : tbl) {
      if (value.first == text) {
        return value.second;
      }
    }
    
    return Layout::CENTER;
  }
  
};


// レイアウト指示のenumを | 演算子で繋げて書く為の定義
View::Layout operator |(const View::Layout lhs, const View::Layout rhs) {
  return static_cast<View::Layout>(lhs + rhs);
}

}
