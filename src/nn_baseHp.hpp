
#pragma once

//
// 白ういろうの体力表示
//


namespace ngs {

class BaseHp {
  const picojson::value& params_;
  int hp_max_;

  bool active_;
  bool finish_;
  
  TextWidget text_;

  MiniEasing<float> mix_ease_;
  int               font_mix_;

  GrpCol col_;
  
  // ダメージ受けた
  bool         damaged_;
  Ease<GrpCol> damaged_ease_;
  
  // 手負い状態
  bool         wounded_;
  Ease<GrpCol> wounded_ease_;

  
public:
  BaseHp(const MatrixFont& font, const picojson::value& params, const Vec2f& ofs, const int hp_max) :
    params_(params),
    hp_max_(hp_max),
    active_(true),
    finish_(false),
    text_(font, params),
    mix_ease_(miniEasingFromJson<float>(params.at("mix_in"))),
    col_(1.0f, 1.0f, 1.0f, 1.0f),
    damaged_(false),
    damaged_ease_(easeFromJson<GrpCol>(params.at("damaged_ease"))),
    wounded_(false),
    wounded_ease_(easeFromJson<GrpCol>(params.at("wounded_ease")))
  {
    // 表示位置の調整
    Vec2f pos = text_.pos() + ofs;
    text_.pos(pos);

    updateText(hp_max);
  }


  void update(const float delta_time) {
    if (!active_) return;
    
    if (mix_ease_.isExec()) {
      font_mix_ = static_cast<int>(mix_ease_(delta_time));
      if (finish_ && !mix_ease_.isExec()) {
        // 表示終了
        active_ = false;
        return;
      }
    }
    
    // 手負い演出
    if (wounded_) {
      col_ = wounded_ease_(delta_time);
    }
    
    // ダメージ演出(手負い演出を上書き)
    if (damaged_) {
      col_ = damaged_ease_(delta_time);
      damaged_ = !damaged_ease_.isEnd();
    }
  }

  void draw(MatrixFont::PrimPack& prim_pack, const View& view) {
    if (!active_) return;

    text_.draw(prim_pack, view, col_, font_mix_, 1);
  }

  
  // 白ういろうがダメージを受けた
  void damage(const int hp) {
    updateText(hp);
    
    damaged_ = true;
    damaged_ease_.toStart();

    // 手負い状態
    if ((static_cast<float>(hp) / hp_max_) <= 0.5f) {
      wounded_ = true;
    }
  }

  // 表示終了
  void finish() {
    if (!active_ || finish_) return;
    
    finish_   = true;
    mix_ease_ = miniEasingFromJson<float>(params_.at("mix_out"));
  }

  
private:
  void updateText(const int hp) {
    static const char tbl[] = "ABCDEFGHIJ";
    text_.text(std::string(&tbl[hp], 1));
  }

};

}
