
#pragma once

//
// 行列フォント
//

#include "co_defines.hpp"
#include <vector>
#include <deque>
#include <unordered_map>
#include <picojson.h>


namespace ngs {

class MatrixFont {
  class Body {
    int width_;
    int height_;
    int height_offset_;

    std::deque<Vec2i> bitmap_;
    
  public:
    explicit Body(const picojson::value& value) :
      width_(0),
      height_offset_(0)
    {
      const auto& columns = value.at("body").get<picojson::array>();

      // Fontの高さ
      height_ = static_cast<int>(columns.size());

      int y = 0;
      for (const auto& it : columns) {
        const std::string& row = it.get<std::string>();
        // Fontの幅(最大長)
        width_ = std::max(static_cast<int>(row.size()), width_);
        int x = 0;
        for (const char c : row) {
          // 一行の中で、空白はドット無し、それ以外はドット有り
          if (c != ' ') {
            bitmap_.push_back(Vec2i(x, y));
          }
          ++x;
        }
        ++y;
      }

      // 表示時の縦のオフセット指定があれば
      if (value.get("height_offset").is<double>()) {
        height_offset_ = static_cast<int>(value.at("height_offset").get<double>());
        DOUT << "height_offset: " << height_offset_ << std::endl;
      }
      
    }

    int width() const { return width_; }
    int height() const { return height_; }
    int heightOffset() const { return height_offset_; }
    const std::deque<Vec2i>& bitmap() const { return bitmap_; }
  };


  // フォントの最大幅
  int width_;
  // フォントの最大高さ
  int height_;
  // 一文字ごとのビットマップデータ
  std::unordered_map<char, Body> bodies_;

  
public:
  // 描画用の頂点の定義
  struct Vtx {
    GLfloat x, y;
  };

  
  explicit MatrixFont(const std::string& filename) :
    width_(0),
    height_(0)
  {
    DOUT << "MatrixFont()" << std::endl;
    
    std::ifstream fstr(filename);
    if (fstr) {
      // ファイルからjsonオブジェクトを生成
      picojson::value json;
      fstr >> json;

      // ルートのオブジェクトを取得
      const picojson::object& objects = json.get<picojson::object>();

      // １文字ずつフォントを生成
      for (const auto& object : objects) {
        Body body(object.second);

        // 空白、改行用にサイズを取得しておく
        width_  = (width_ + body.width()) / 2;
        height_ = std::max(width_, body.height());

        // 生成したデータを連想記憶に積む
        char name = object.first[0];
        bodies_.insert(std::unordered_map<char, Body>::value_type(name, body));
        DOUT << name << ": " << body.width() << "x" << body.height() << std::endl;
      }
    }
    DOUT << "Font: " << width_ << "x" << height_ << std::endl;
  }

  ~MatrixFont() {
    DOUT << "~MatrixFont()" << std::endl;
  }


  // 表示サイズを返す
  Vec2f getTextSize(const std::string& text) const {
    Vec2f size(Vec2f::Zero());

    float x = 0.0f;
    for (const char chara : text) {
      switch (chara) {
      case ' ':
        // 空白
        x += width_;
        break;
        
      case '\n':
        // 改行
        size.x() = std::max(size.x(), x - 1.0f);
        size.y() += height_;
        x = 0;
        break;

      default:
        {
          const Body& body = bodies_.at(chara);
          x += body.width() + 1.0f;
        }
        break;
      }
    }
    // 最後の行の幅と高さを忘れずに
    size.x() = std::max(size.x(), x - 1.0f);
    if (x > 0.0f) size.y() += height_;

    return size;
  }


  // 描画プリミティブ
  struct Prim {
    Mat4f matrix;
    GrpCol color;
    size_t index;
    size_t num;
  };
  
  struct PrimPack {
    std::vector<Vtx> vtxes;
    std::deque<Prim> prims;
  };

  
  // 描画プリミティブを生成する
  void createTextPrim(PrimPack& prim_pack, const std::string& text, const Mat4f& matrix, const GrpCol& color,
                      const int mix = 0, const int mix_increase = 0) const {
    Prim prim = {
      matrix,
      color,
      prim_pack.vtxes.size()
    };

    // stringから一文字ごとに頂点データを生成
    float x = 0.0f;
    float y = 0.0f;
    int mix_current = mix;
    for (const char chara : text) {
      switch (chara) {
      case ' ':
        // 空白
        x += width_;
        break;
        
      case '\n':
        // 改行
        x  = 0.0f;
        y -= height_ + 1;
        break;

      default:
        {
          const Body& body = bodies_.at(chara);
          makeCharaPrim(prim_pack.vtxes, x, y + body.heightOffset(), body, std::max(mix_current, 0));
          x += body.width() + 1.0f;
          mix_current += mix_increase;
        }
        break;
      }
    }

    // 生成した頂点数を記録
    prim.num = prim_pack.vtxes.size() - prim.index;
    prim_pack.prims.push_back(prim);
  }


private:
  // 一文字の頂点データ生成
  void makeCharaPrim(std::vector<Vtx>& vtxes, const float start_x, const float start_y, const Body& body, const int mix) const {
    int width  = body.width();
    int height = body.height();
    const auto& bitmap = body.bitmap();

    int mix_x = mix;
    int mix_y = mix / width;
    
    for (const auto& pixel : bitmap) {
      float x = (pixel.x() + mix_x) % width + start_x;
      float y = -((pixel.y() + mix_y / width) % height) + start_y;
      
      // 一辺0.8な正方形を描画
      Vtx vtx[] = {
        { x,        y        },
        { x + 0.8f, y        },
        { x,        y - 0.8f },
            
        { x + 0.8f, y        },
        { x,        y - 0.8f },
        { x + 0.8f, y - 0.8f }
      };
      // コンテナの末尾にまとめて追加
      vtxes.insert(vtxes.end(), vtx, &vtx[ELEMSOF(vtx)]);

      if (mix) {
        mix_x += mix;
        mix_y += mix / width;
      }
    }
  }
  
};

}
