
#pragma once

//
// テキストを簡便に表示したりする
//

#include "co_vector.hpp"
#include "co_matrix.hpp"
#include "co_view.hpp"
#include "co_json.hpp"
#include "nn_matrixFont.hpp"


namespace ngs {

class TextWidget {
public:
  enum Layout {
    CENTER = 0,
    TOP    = (1 << 0),
    BOTTOM = (1 << 1),
    LEFT   = (1 << 2),
    RIGHT  = (1 << 3)
  };

  
private:
  const MatrixFont& font_;
  std::string text_;

  Vec2f  pos_;
  float  scale_;
  Layout layout_;

  View::Layout view_layout_;

  Vec2f size_;
  Vec2f disp_pos_;
  
  bool recalc_pos_;
  bool recalc_size_;


public:
  TextWidget(const MatrixFont& font,
             const std::string& text,
             const Vec2f& pos, const float scale, const Layout layout,
             const View::Layout view_layout) :
    font_(font),
    text_(text),
    pos_(pos),
    scale_(scale),
    layout_(layout),
    view_layout_(view_layout),
    size_(font.getTextSize(text) * scale),
    disp_pos_(layoutPos(pos, size_, layout)),
    recalc_pos_(false),
    recalc_size_(false)
  {
    DOUT << "TextWidget()" << std::endl;
  }

  TextWidget(const MatrixFont& font, const picojson::value& json) :
    font_(font),
    text_(json.at("text").get<std::string>()),
    pos_(vectFromJson<Vec2f>(json.at("pos"))),
    scale_(json.at("scale").get<double>()),
    layout_(layoutFromString(json.at("layout").get<std::string>())),
    view_layout_(View::layoutFromString(json.at("view_layout").get<std::string>())),
    size_(font.getTextSize(text_) * scale_),
    disp_pos_(layoutPos(pos_, size_, layout_)),
    recalc_pos_(false),
    recalc_size_(false)
  {
    DOUT << "TextWidget()" << std::endl;
  }

  ~TextWidget() {
    DOUT << "~TextWidget()" << std::endl;
  }

  
  void text(const std::string& text) {
    text_ = text;

    // 表示位置、サイズ変更の再計算を要求
    recalc_pos_  = true;
    recalc_size_ = true;
  }

  const std::string& text() const { return text_; }

  const Vec2f& pos() const { return pos_; }
  
  void pos(const Vec2f& pos) {
    pos_ = pos;

    // 表示位置変更の再計算を要求
    recalc_pos_ = true;
  }

  const Vec2f& dispPos() {
    if (recalc_pos_ || recalc_size_) {
      // 文字列やスケーリングが書き換えられていたら表示位置を再計算
      disp_pos_ = layoutPos(pos_, size(), layout_);

      recalc_pos_  = false;
      recalc_size_ = false;
    }
    return disp_pos_;
  }

  const Vec2f& size() {
    if (recalc_size_) {
      // 文字列やスケーリングが書き換えられていたらサイズを再計算
      size_ = font_.getTextSize(text_) * scale_;
      recalc_size_ = false;
    }
    return size_;
  }

  void scale(const float scale) {
    scale_ = scale;

    // 表示位置、サイズ変更の再計算を要求
    recalc_pos_  = false;
    recalc_size_ = true;
  }
  
  float scale() const { return scale_; }

  void layout(const Layout layout) { layout_ = layout; }
  void viewLayout(const View::Layout layout) { view_layout_ = layout; }
  View::Layout viewLayout() const { return view_layout_; }


  // 描画プリミティブを生成してコンテナに積む
  void draw(MatrixFont::PrimPack& prim_pack,
            const View& view, const GrpCol& col, const int mix = 0, const int mix_increase = 0) {
    // 表示位置を決定
    Vec2f pos = view.layoutPos(dispPos(), view_layout_);

    // TIPS:表示座標は小数点切り捨て
    // 行列を作成
    Eigen::Affine3f m =
      Eigen::Translation<float, 3>(Vec3f(std::floor(pos.x()), std::floor(pos.y()), 0.0f))
      * Eigen::Scaling(Vec3f(scale_, scale_, 1.0f));

    // 描画プリミティブ生成
    font_.createTextPrim(prim_pack, text_, m.matrix(), col, mix, mix_increase);
  }
  
private:
  Vec2f layoutPos(const Vec2f& pos, const Vec2f& size, Layout type = CENTER) const {
    Vec2f layouted_pos = pos;

    if (!(type & (TOP | BOTTOM))) layouted_pos.y() += size.y() / 2;
    else if (type & BOTTOM)          layouted_pos.y() += size.y();
    if (!(type & (LEFT | RIGHT))) layouted_pos.x() -= size.x() / 2;
    else if (type & RIGHT)        layouted_pos.x() -= size.x();

    return layouted_pos;
  }


public:
  
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
TextWidget::Layout operator |(const TextWidget::Layout lhs, const TextWidget::Layout rhs) {
  return static_cast<TextWidget::Layout>(lhs + rhs);
}

}
