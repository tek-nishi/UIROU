
#pragma once

//
// タイトルロゴ表示
//

#include "nn_textWidget.hpp"
#include "nn_touchWidget.hpp"
#include "nn_menuMisc.hpp"
#include "nn_gameSound.hpp"
#include "gamecenter.h"
#include "advertisement.h"


namespace ngs {

class TitleLogo : public ObjBase {
  Framework& fw_;
  const picojson::value& params_;
  TouchWidget& touch_widget_;

  bool active_;
  bool updated_;

  bool bgm_active_;
  bool se_active_;
  
  enum Mode {
    INTERVAL,
    PREFACE_IN,
    PREFACE,
    PREFACE_OUT,
    LOGO_IN,
    WAIT_TAP,
    LOGO_OUT
  };
  Mode mode_;
  float intarval_;

  float preface_duration_;
  
  TextWidget preface_;
  TextWidget logo_;
  TextWidget logo_sub_;
#ifdef VER_LITE
  TextWidget logo_lite_;
#endif
  TextWidget copyright_;
  TextWidget start_;
  TextWidget top_text_;
  TextWidget product_no_;

  TextWidget bgm_;
  TextWidget bgm_text_;
  TextWidget se_;
  TextWidget se_text_;
  TextWidget records_;
  TextWidget records_text_;
  TextWidget credits_;
  TextWidget credits_text_;

  bool active_records_;
  
  TouchWidget::Handle bgm_handle_;
  TouchWidget::Handle se_handle_;
  TouchWidget::Handle records_handle_;
  TouchWidget::Handle credits_handle_;
  TouchWidget::Handle start_handle_;

  bool on_bgm_;
  bool on_se_;
  bool on_records_;
  bool on_credits_;
  bool on_start_;

  // BGMとSEのOFFの色
  GrpCol icon_off_;

#if defined (USE_GAMECENTER)
  TextWidget          gamecenter_;
  TextWidget          gamecenter_text_;
  TouchWidget::Handle gamecenter_handle_;
  bool                on_gamecenter_;
#endif

  MiniEasing<float> mix_ease_;
  int logo_mix_;

  Ease<GrpCol> start_ease_color_;
  GrpCol start_color_;

  float tap_text_time_;

  enum CloseMode {
    EXEC_GAME,
    EXEC_RECORDS,
    EXEC_CREDITS,
  };
  CloseMode close_mode_;

  float demo_start_time_;
  
  // 宣伝ダイアログを表示[iOS]
  bool advertisement_;
  
  
public:
  TitleLogo(Framework& fw, const picojson::value& params,
            const picojson::value& settings,
            const MatrixFont& font, const MatrixFont& kana_font, const MatrixFont& icon_font,
            TouchWidget& touch_widget) :
    fw_(fw),
    params_(params.at("title")),
    touch_widget_(touch_widget),
    active_(true),
    updated_(false),
    mode_(INTERVAL),
    intarval_(params_.at("interval").get<double>()),
    preface_(font, params_.at("preface")),
    logo_(kana_font, params_.at("logo")),
    logo_sub_(font, params_.at("logo_sub")),
#ifdef VER_LITE
    logo_lite_(font, params_.at("lite")),
#endif
    copyright_(font, params_.at("copyright")),
    start_(font, params_.at("start")),
    top_text_(font, params_.at("top")),
    product_no_(font, params_.at("product_no")),
    bgm_(icon_font, params_.at("bgm")),
    se_(icon_font, params_.at("se")),
    bgm_text_(font, params_.at("bgm_text")),
    se_text_(font, params_.at("se_text")),
    records_(icon_font, params_.at("records")),
    records_text_(font, params_.at("records_text")),
    credits_(icon_font, params_.at("credits")),
    credits_text_(font, params_.at("credits_text")),
    on_bgm_(false),
    on_se_(false),
    on_records_(false),
    on_credits_(false),
    on_start_(false),
    icon_off_(vectFromJson<GrpCol>(params_.at("icon_off"))),
#if defined (USE_GAMECENTER)
    gamecenter_(icon_font, params_.at("gamecenter")),
    gamecenter_text_(font, params_.at("gamecenter_text")),
    on_gamecenter_(false),
#endif
    mix_ease_(miniEasingFromJson<float>(params_.at("preface_mix_in"))),
    start_ease_color_(easeFromJson<GrpCol>(params_.at("start_color"))),
    start_color_(0.0f, 0.0f, 0.0f, 0.0f),
    tap_text_time_(0.0f),
    demo_start_time_(params_.at("demo_start_time").get<double>()),
    advertisement_(true)
  {
    DOUT << "TitleLogo()" << std::endl;

    // 演出時間はプレイ回数が多いと短くなる
    MiniEasing<float> disp_time(miniEasingFromJson<float>(params_.at("preface_disp_time")));
    float play_num = static_cast<float>(settings.at("number_of_played").get<double>());
    preface_duration_ = disp_time.at(play_num);
    
    start_.text(params_.at("start_text").get(0).get<std::string>());

    {
      // BESTスコア
      std::ostringstream text;
      text << top_text_.text() << std::setw(6) << std::setfill('0') << int(settings.at("best_score").get<double>());
      top_text_.text(text.str());
    }
    
    // タッチ範囲をコールバックに登録
    setupWidgetCallback();
  }

  ~TitleLogo() {
    DOUT << "~TitleLogo()" << std::endl;

    touch_widget_.removeWidget(bgm_handle_);
    touch_widget_.removeWidget(se_handle_);
    touch_widget_.removeWidget(records_handle_);
    touch_widget_.removeWidget(credits_handle_);
    touch_widget_.removeWidget(start_handle_);
#if defined (USE_GAMECENTER)
    touch_widget_.removeWidget(gamecenter_handle_);
#endif
  }

  
  bool isActive() const {
    return active_;
  }
  
  void message(const int msg, Signal::Params& arguments) {
    if (!active_) return;
    
    switch (msg) {
    case Msg::UPDATE:
      update(arguments);
      return;

    case Msg::DRAW:
      draw(arguments);
      return;

      
    case Msg::SET_SPAWN_INFO:
      setupFromSpawnInfo(arguments);
      return;

      
    case Msg::FINISH_SKIP_TAP:
      skipPreface();
      return;

      
    case Msg::START_GAME:
      active_ = false;
      return;

      
    case Msg::EXEC_DEMO_MODE:
      active_ = false;
      return;

      
    default:
      return;
    }
  }

  
private:
  // 更新
  void update(const Signal::Params& arguments) {
    updated_ = true;

    float delta_time = boost::any_cast<float>(arguments.at("delta_time"));

    switch (mode_) {
    case INTERVAL:
      // 最初の間
      intarval_ -= delta_time;
      if (intarval_ < 0.0f) {
        mode_     = PREFACE_IN;
        logo_mix_ = mix_ease_.startValue();
        // SE
        gamesound::play(fw_, "intro");
      }
      break;
      
    case PREFACE_IN:
      // 序文表示演出
      logo_mix_ = static_cast<int>(mix_ease_(delta_time));
      if (!mix_ease_.isExec()) {
        mode_     = PREFACE;
        intarval_ = preface_duration_;
      }
      break;

    case PREFACE:
      // 序文表示
      intarval_ -= delta_time;
      if (intarval_ <= 0.0f) {
        mode_     = PREFACE_OUT;
        mix_ease_ = miniEasingFromJson<float>(params_.at("preface_mix_out"));
        logo_mix_ = mix_ease_.startValue();
      }
      break;

    case PREFACE_OUT:
      // 序文消去演出
      logo_mix_ = static_cast<int>(mix_ease_(delta_time));
      if (!mix_ease_.isExec()) {
        mode_     = LOGO_IN;
        mix_ease_ = miniEasingFromJson<float>(params_.at("logo_mix_in"));
        // SE
        gamesound::play(fw_, "title");
      }
      break;

    case LOGO_IN:
      // タイトルロゴ表示演出
      logo_mix_ = static_cast<int>(mix_ease_(delta_time));
      if (!mix_ease_.isExec()) {
        // 表示演出が終わったらタップ待ち
        mode_     = WAIT_TAP;
        logo_mix_ = 0.0f;
        
        // Sound設定入力受付開始
        touch_widget_.activeWidget(bgm_handle_, true);
        touch_widget_.activeWidget(se_handle_, true);
        if (active_records_) {
          // 記録画面は1回以上プレイしてから
          touch_widget_.activeWidget(records_handle_, true);
        }
        touch_widget_.activeWidget(credits_handle_, true);
        touch_widget_.activeWidget(start_handle_, true);
#if defined (USE_GAMECENTER)
        touch_widget_.activeWidget(gamecenter_handle_, true);
#endif
        {
          // Skip操作中断
          Signal::Params params;
          fw_.signal().sendMessage(Msg::ABORT_SKIP_TAP, params);
        }

        // 宣伝ダイアログ[iOS]
        if (advertisement_) advertisement::popup();
      }
      break;

    case WAIT_TAP:
      // タップ待ち
      {
        // テキストの明滅
        start_color_ = start_ease_color_(delta_time);

        // テキストアニメーション
        tap_text_time_ += delta_time;

        float speed = params_.at("start_tap_speed").get<double>();
        if (on_start_) speed *= 1.5f;

        const auto& text = params_.at("start_text").get<picojson::array>();
        start_.text(text[int(tap_text_time_ * speed) % text.size()].get<std::string>());

        demo_start_time_ -= delta_time;
        if (demo_start_time_ <= 0.0f) {
          // DEMO開始
          Signal::Params params;
          fw_.signal().sendMessage(Msg::FORCE_PLAYBACK_MODE, params);
          fw_.signal().sendMessage(Msg::EXEC_DEMO_MODE, params);
        }
      }
      break;

    case LOGO_OUT:
      logo_mix_ = static_cast<int>(mix_ease_(delta_time));
      if (!mix_ease_.isExec()) {
        // ロゴ消去演出が終わったら次の画面へ
        switch (close_mode_) {
        case EXEC_GAME:
          {
            Signal::Params params;
            fw_.signal().sendMessage(Msg::START_GAME, params);
          }
          break;

        case EXEC_CREDITS:
          {
            Signal::Params params;
            fw_.signal().sendMessage(Msg::START_CREDITS, params);
          }
          break;

        case EXEC_RECORDS:
          {
            Signal::Params params;
            fw_.signal().sendMessage(Msg::START_RECORDS, params);
          }
          break;
        }
        active_ = false;
        return;
      }
      break;
    }

#ifdef _DEBUG
    char key = fw_.keyboard().getPushed();
    if (key == 'r') {
      // 記録モード
      Signal::Params params;
      fw_.signal().sendMessage(Msg::TOGGLE_RECORD_MODE, params);
    }
    else if (key == 'p'){
      // 再生モード
      Signal::Params params;
      fw_.signal().sendMessage(Msg::TOGGLE_PLAYBACK_MODE, params);
    }
    else if (key == 'd') {
      // デモ開始
      Signal::Params params;
      fw_.signal().sendMessage(Msg::EXEC_DEMO_MODE, params);
    }
#endif
    
  }

  // 描画
  void draw(Signal::Params& arguments) {
    if (!updated_) return;

    // 描画プリミティブの格納先をポインタで受け取る
    auto prims = boost::any_cast<MatrixFont::PrimPack*>(arguments.at("text_prims"));
      
    switch (mode_) {
    case PREFACE_IN:
    case PREFACE:
    case PREFACE_OUT:
      // 序文表示
      drawPreface(*prims);
      break;

    case LOGO_IN:
    case WAIT_TAP:
    case LOGO_OUT:
      // タイトルロゴ表示
      drawLogo(*prims);
      break;

    default:
      break;
    }
  }

  
  // 序文表示
  void drawPreface(MatrixFont::PrimPack& prims) {
    preface_.draw(prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f), logo_mix_ - 27, 1);
  }

  // ロゴなどの表示
  void drawLogo(MatrixFont::PrimPack& prims) {
    logo_.draw(prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f), logo_mix_ - 25, 1);
    logo_sub_.draw(prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f), logo_mix_ - 20, 1);
#ifdef VER_LITE
    logo_lite_.draw(prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f), logo_mix_ - 15, 1);
#endif

    {
      GrpCol col = on_start_ ? GrpCol(1.0f, 0.0f, 0.0f, 1.0f) : start_color_;
      start_.draw(prims, fw_.view(), col, logo_mix_ - 11, 1);
    }
    copyright_.draw(prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f), logo_mix_ - 26, 1);
    product_no_.draw(prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f), logo_mix_ - 22, 1);

    {
      // BGM ON/OFF は状況で表示色を変更する
      GrpCol col = bgm_active_ ? GrpCol(1.0f, 1.0f, 1.0f, 1.0f) : icon_off_;
      if (on_bgm_) col = GrpCol(1.0f, 0.0f, 0.0f, 1.0f);
      bgm_.draw(prims, fw_.view(), col, logo_mix_ - 1, 1);
      bgm_text_.draw(prims, fw_.view(), col, logo_mix_ - 4, 1);
    }

    {
      // SE ON/OFF は状況で表示色を変更する
      GrpCol col = se_active_ ? GrpCol(1.0f, 1.0f, 1.0f, 1.0f) : icon_off_;
      if (on_se_) col = GrpCol(1.0f, 0.0f, 0.0f, 1.0f);
      se_.draw(prims, fw_.view(), col, logo_mix_ - 2, 1);
      se_text_.draw(prims, fw_.view(), col, logo_mix_ - 4, 1);
    }

    if (active_records_) {
      top_text_.draw(prims, fw_.view(), GrpCol(1.0f, 1.0f, 1.0f, 1.0f), logo_mix_ - 15, 1);
      
      // Records は状況で表示色を変更する
      GrpCol col = on_records_ ? GrpCol(1.0f, 0.0f, 0.0f, 1.0f) : GrpCol(1.0f, 1.0f, 1.0f, 1.0f);
      records_.draw(prims, fw_.view(), col, logo_mix_ - 3, 1);
      records_text_.draw(prims, fw_.view(), col, logo_mix_ - 10, 1);
    }

    {
      // Credits は状況で表示色を変更する
      GrpCol col = on_credits_ ? GrpCol(1.0f, 0.0f, 0.0f, 1.0f) : GrpCol(1.0f, 1.0f, 1.0f, 1.0f);
      credits_.draw(prims, fw_.view(), col, logo_mix_ - 4, 1);
      credits_text_.draw(prims, fw_.view(), col, logo_mix_ - 10, 1);
    }

#if defined (USE_GAMECENTER)
    // GameCenter は状況で表示色を変更する
    GrpCol col = on_gamecenter_ ? GrpCol(1.0f, 0.0f, 0.0f, 1.0f) : GrpCol(1.0f, 1.0f, 1.0f, 1.0f);
    gamecenter_.draw(prims, fw_.view(), col, logo_mix_ - 4, 1);
    gamecenter_text_.draw(prims, fw_.view(), col, logo_mix_ - 10, 1);
#endif
  }


  // 生成情報をもとに初期設定を行う
  void setupFromSpawnInfo(const Signal::Params& arguments) {
    bgm_active_ = boost::any_cast<bool>(arguments.at("bgm_active"));
    bgmText(bgm_active_);
      
    se_active_ = boost::any_cast<bool>(arguments.at("se_active"));
    seText(se_active_);

    active_records_ = boost::any_cast<bool>(arguments.at("active_records"));
      
    if (boost::any_cast<bool>(arguments.at("start_logo"))) {
      // 途中から開始
      mode_     = LOGO_IN;
      mix_ease_ = miniEasingFromJson<float>(params_.at("logo_mix_in"));

      // 他の画面から戻ってきた時は宣伝ダイアログを表示しない[iOS]
      advertisement_ = false;
    }
  }

  // 演出飛ばし
  void skipPreface() {
    switch (mode_) {
    case INTERVAL:
    case PREFACE_IN:
    case PREFACE:
    case PREFACE_OUT:
      // 序文スキップ(表示演出をカットする)
      mode_     = LOGO_IN;
      mix_ease_ = miniEasingFromJson<float>(params_.at("logo_mix_in"));
      mix_ease_.toEnd();

      {
        // カメラ演出強制終了
        Signal::Params params;
        fw_.signal().sendMessage(Msg::TO_END_EASE_CAMERA, params);
      }

      // SE
      gamesound::play(fw_, "title");
      break;
        
    case LOGO_IN:
      // ロゴ演出スキップ(表示演出をカットする)
      mix_ease_.toEnd();

      {
        // カメラ演出強制終了
        Signal::Params params;
        fw_.signal().sendMessage(Msg::TO_END_EASE_CAMERA, params);
      }
      break;

    default:
      break;
    }
  }
  
  
  void bgmText(const bool active) {
    bgm_.text(active ? "B" : "b");
  }
  
  void seText(const bool active) {
    se_.text(active ? "S" : "s");
  }


  // ボタン類のセットアップ
  void setupWidgetCallback() {
    // FIXME:boost::_1と被るので、名前空間を省略できない
    bgm_handle_ = menu::addWidget(touch_widget_, bgm_,
                                  std::bind(&TitleLogo::bgmWidget,
                                            this, std::placeholders::_1, std::placeholders::_2), false);

    se_handle_ = menu::addWidget(touch_widget_, se_,
                                 std::bind(&TitleLogo::seWidget,
                                           this, std::placeholders::_1, std::placeholders::_2), false);

    records_handle_ = menu::addWidget(touch_widget_, records_,
                                      std::bind(&TitleLogo::recordsWidget,
                                                this, std::placeholders::_1, std::placeholders::_2), false);
    
    credits_handle_ = menu::addWidget(touch_widget_, credits_,
                                      std::bind(&TitleLogo::creditsWidget,
                                                this, std::placeholders::_1, std::placeholders::_2), false);

    start_handle_ = menu::addWidget(touch_widget_, start_,
                                    std::bind(&TitleLogo::startWidget,
                                              this, std::placeholders::_1, std::placeholders::_2), false);
#if defined (USE_GAMECENTER)
    gamecenter_handle_ = menu::addWidget(touch_widget_, gamecenter_,
                                         std::bind(&TitleLogo::gamecenterWidget,
                                                   this, std::placeholders::_1, std::placeholders::_2), false);
#endif
  }

  
  // BGM ON/OFF
  void bgmWidget(const TouchWidget::Msg msg, const Vec2f& pos) {
    switch (msg) {
    case TouchWidget::TOUCH_START:
      on_bgm_ = true;
      break;
      
    case TouchWidget::MOVE_OUT_IN:
      on_bgm_ = true;
      break;
      
    // case TouchWidget::MOVE_IN:
    //   break;
      
    case TouchWidget::MOVE_IN_OUT:
      on_bgm_ = false;
      break;
      
    // case TouchWidget::MOVE_OUT:
    //   break;
      
    case TouchWidget::TOUCH_END_IN:
      on_bgm_ = false;
      bgm_active_ = !bgm_active_;
      changeSoundSettings();
      break;
      
    case TouchWidget::TOUCH_END_OUT:
      on_bgm_ = false;
      break;

    default:
      break;
    }
  }
  
  // SE ON/OFF
  void seWidget(const TouchWidget::Msg msg, const Vec2f& pos) {
    switch (msg) {
    case TouchWidget::TOUCH_START:
      on_se_ = true;
      break;
      
    case TouchWidget::MOVE_OUT_IN:
      on_se_ = true;
      break;
      
    // case TouchWidget::MOVE_IN:
    //   break;
      
    case TouchWidget::MOVE_IN_OUT:
      on_se_ = false;
      break;
      
    // case TouchWidget::MOVE_OUT:
    //   break;
      
    case TouchWidget::TOUCH_END_IN:
      on_se_ = false;
      se_active_ = !se_active_;
      changeSoundSettings();
      break;
      
    case TouchWidget::TOUCH_END_OUT:
      on_se_ = false;
      break;

    default:
      break;
    }
  }
  
  // 記録画面
  void recordsWidget(const TouchWidget::Msg msg, const Vec2f& pos) {
    switch (msg) {
    case TouchWidget::TOUCH_START:
      {
        on_records_ = true;

        // 照準操作は中断
        Signal::Params params;
        fw_.signal().sendMessage(Msg::MANIPULATE_ONCE_SKIP, params);
      }
      break;
      
    case TouchWidget::MOVE_OUT_IN:
      on_records_ = true;
      break;
      
    // case TouchWidget::MOVE_IN:
    //   break;
      
    case TouchWidget::MOVE_IN_OUT:
      on_records_ = false;
      break;
      
    // case TouchWidget::MOVE_OUT:
    //   break;
      
    case TouchWidget::TOUCH_END_IN:
      {
        // タイトルを終了して記録画面を起動
        // on_records_ = false;
        closeTitle(EXEC_RECORDS);
        gamesound::play(fw_, "wipe");
      }
      break;
      
    case TouchWidget::TOUCH_END_OUT:
      on_records_ = false;
      break;

    default:
      break;
    }
  }
  
  // クレジット画面
  void creditsWidget(const TouchWidget::Msg msg, const Vec2f& pos) {
    switch (msg) {
    case TouchWidget::TOUCH_START:
      {
        on_credits_ = true;

        // 照準操作は中断
        Signal::Params params;
        fw_.signal().sendMessage(Msg::MANIPULATE_ONCE_SKIP, params);
      }
      break;
      
    case TouchWidget::MOVE_OUT_IN:
      on_credits_ = true;
      break;
      
    // case TouchWidget::MOVE_IN:
    //   break;
      
    case TouchWidget::MOVE_IN_OUT:
      on_credits_ = false;
      break;
      
    // case TouchWidget::MOVE_OUT:
    //   break;
      
    case TouchWidget::TOUCH_END_IN:
      {
        // タイトルを終了して記録画面を起動
        // on_credits_ = false;
        closeTitle(EXEC_CREDITS);
        gamesound::play(fw_, "wipe");
      }
      break;
      
    case TouchWidget::TOUCH_END_OUT:
      on_credits_ = false;
      break;

    default:
      break;
    }
  }

  // GAME START
  void startWidget(const TouchWidget::Msg msg, const Vec2f& pos) {
    switch (msg) {
    case TouchWidget::TOUCH_START:
      {
        on_start_      = true;
        tap_text_time_ = 0.0f;

        // 照準操作は中断
        Signal::Params params;
        fw_.signal().sendMessage(Msg::MANIPULATE_ONCE_SKIP, params);
      }
      break;
      
    case TouchWidget::MOVE_OUT_IN:
      on_start_      = true;
      tap_text_time_ = 0.0f;
      break;
      
    // case TouchWidget::MOVE_IN:
    //   break;
      
    case TouchWidget::MOVE_IN_OUT:
      on_start_ = false;
      start_.text(params_.at("start_text").get(0).get<std::string>());
      break;
      
    // case TouchWidget::MOVE_OUT:
    //   break;
      
    case TouchWidget::TOUCH_END_IN:
      {
        // ゲーム開始
        // on_start_ = false;
        closeTitle(EXEC_GAME);
        gamesound::stopAll(fw_);
        gamesound::play(fw_, "start");
      }
      break;
      
    case TouchWidget::TOUCH_END_OUT:
      on_start_ = false;
      start_.text(params_.at("start_text").get(0).get<std::string>());
      break;

    default:
      break;
    }
  }

  
#if defined (USE_GAMECENTER)
  // GameCenter
  void gamecenterWidget(const TouchWidget::Msg msg, const Vec2f& pos) {
    switch (msg) {
    case TouchWidget::TOUCH_START:
      {
        on_gamecenter_ = true;

        // 照準操作は中断
        Signal::Params params;
        fw_.signal().sendMessage(Msg::MANIPULATE_ONCE_SKIP, params);
      }
      break;
      
    case TouchWidget::MOVE_OUT_IN:
      on_gamecenter_ = true;
      break;
      
    // case TouchWidget::MOVE_IN:
    //   break;
      
    case TouchWidget::MOVE_IN_OUT:
      on_gamecenter_ = false;
      break;
      
    // case TouchWidget::MOVE_OUT:
    //   break;
      
    case TouchWidget::TOUCH_END_IN:
      {
        // GameCenter画面表示
        on_gamecenter_ = false;
        gamecenter::showGamecenter();
        gamesound::play(fw_, "wipe");
      }
      break;
      
    case TouchWidget::TOUCH_END_OUT:
      on_gamecenter_ = false;
      break;

    default:
      break;
    }
  }
#endif

  
  // タイトル終了演出開始
  void closeTitle(const CloseMode mode) {
    close_mode_ = mode;

    mode_     = LOGO_OUT;
    mix_ease_ = miniEasingFromJson<float>(params_.at("logo_mix_out"));

    touch_widget_.activeWidget(bgm_handle_, false);
    touch_widget_.activeWidget(se_handle_, false);
    touch_widget_.activeWidget(records_handle_, false);
    touch_widget_.activeWidget(credits_handle_, false);
    touch_widget_.activeWidget(start_handle_, false);
#if defined (USE_GAMECENTER)
    touch_widget_.activeWidget(gamecenter_handle_, false);
#endif
  }
  

  // シグナル方式でサウンド設定の変更
  void changeSoundSettings() {
    bgmText(bgm_active_);
    seText(se_active_);

    Signal::Params params;
    params.insert(Signal::Params::value_type("bgm_active", bgm_active_));
    params.insert(Signal::Params::value_type("se_active", se_active_));
    fw_.signal().sendMessage(Msg::CHANGE_SOUND_SETTIGS, params);
  }
  
};

}
