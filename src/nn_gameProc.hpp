
#pragma once

//
// ゲーム本体
//

#include "co_defines.hpp"
#include <algorithm>
#include <list>
#include <sstream>
#include <iterator>
#include "co_json.hpp"
#include "co_procBase.hpp"
#include "co_zlib.hpp"
#include "nn_camera.hpp"
#include "nn_light.hpp"
#include "nn_modelHolder.hpp"
#include "nn_shaderHolder.hpp"
#include "nn_messages.hpp"
#include "nn_matrixFont.hpp"
#include "nn_manipulate.hpp"
#include "nn_easeCamera.hpp"
#include "nn_bg.hpp"
#include "nn_planet.hpp"
#include "nn_space.hpp"
#include "nn_cubeBase.hpp"
#include "nn_cubePlayer.hpp"
#include "nn_cubeEnemy.hpp"
#include "nn_cubeItem.hpp"
#include "nn_cpuFactory.hpp"
#include "nn_generator.hpp"
#include "nn_signt.hpp"
#include "nn_titleLogo.hpp"
#include "nn_records.hpp"
#include "nn_credits.hpp"
#include "nn_gameLogic.hpp"
#include "nn_gameStart.hpp"
#include "nn_gameOver.hpp"
#include "nn_gameResult.hpp"
#include "nn_pauseMenu.hpp"
#include "nn_quakeCamera.hpp"
#include "nn_touchWidget.hpp"
#include "nn_attackEffect.hpp"
#include "nn_awaitTap.hpp"
#include "nn_skipTap.hpp"
#include "nn_cubeShadow.hpp"
#include "nn_gameSound.hpp"
#include "nn_settings.hpp"
#include "nn_demoLogic.hpp"
#include "sns.h"
#include "gamecenter.h"


namespace ngs {

class GameProc : public ProcBase {
  Framework& fw_;
  const picojson::value params_;
  const picojson::value& game_params_;
  Settings settings_;
  
  Camera      camera_;
  Camera      camera_2d_;
  QuakeCamera quake_camera_;

  std::pair<Vec3f, Light> lights_[3];

#ifdef _DEBUG
  bool draw_text_only_;
  bool draw_text_;
#endif

  bool input_record_;
  bool input_playback_;
  bool fix_framerate_;
  
  // Modelの簡単なキャッシュ
  ModelHolder model_holder_;
  // シェーダーの簡単なキャッシュ
  ShaderHolder shader_holder_;
  // ういろうの影
  CubeShadow shadow_;
  
  // メニュー操作
  TouchWidget touch_widget_;

  Signal::Handle signal_handle_;
  
  // ゲーム内オブジェクト
  std::deque<std::shared_ptr<ObjBase> > objects_;

  // 惑星の半径
  float planet_radius_;
  // 敵CPU
  CpuFactory cpu_factory_;
  
  // Font
  MatrixFont font_;
  MatrixFont number_font_;
  MatrixFont kana_font_;
  MatrixFont icon_font_;
  Vbo        font_vbo_;

  MatrixFont::PrimPack text_prims_;
  
  bool pause_;

  
public:
  explicit GameProc(Framework& fw) :
    fw_(fw),
    params_(readJsonFile(fw.loadPath(), "params.json")),
    game_params_(params_.at("game")),
    settings_(fw, "settings"),
    camera_(fw_.view().size(), deg2rad(game_params_.at("camera_angle").get<double>()),
            game_params_.at("camera_near_z").get<double>(),
            game_params_.at("camera_far_z").get<double>()),
    camera_2d_(fw.view().size(), 1.0f),
#ifdef _DEBUG
    draw_text_only_(false),
    draw_text_(true),
#endif
    input_record_(false),
    input_playback_(false),
    fix_framerate_(false),
    model_holder_(fw.loadPath()),
    shader_holder_(fw.loadPath()),
    planet_radius_(game_params_.at("planet_radius").get<double>()),
    font_(fw.loadPath() + "font.json"),
    number_font_(fw.loadPath() + "number.json"),
    kana_font_(fw.loadPath() + "kana.json"),
    icon_font_(fw.loadPath() + "icon.json"),
    shadow_(fw_.loadPath() + "shadow.dae", fw_.loadPath() + "shadow.png", shader_holder_),
    touch_widget_(fw),
    pause_(false)
  {
    DOUT << "GameProc()" << std::endl;

    // テキスト描画用のバッファを予約
    text_prims_.vtxes.reserve(2048);
    
    // サウンド
    Audio::listenerPosition(Vec3f::Zero());
    fw.readSound(gamesound::readFromJson(params_.at("sound"), fw.loadPath()));
    gamesound::active(settings_.value().at("bgm").get<bool>(), settings_.value().at("se").get<bool>());
    
    // タッチ判定の余白を設定
    touch_widget_.margin(game_params_.at("margin").get<double>());
    
    // 描画バッファの消去色を指示
    fw_.view().clearColor(vectFromJson<GrpCol>(game_params_.at("clear_color")));

    // 光源設定
    initLights(game_params_.at("lights"));
    
    // カメラ設定
		camera_.eye() = vectFromJson<Vec3f>(game_params_.at("camera_eye"));

    // 自分自身をシグナルに登録
    signal_handle_ = fw_.signal().connect(*this);

    // 操作オブジェクト
    spawnObject<Manipulate>(fw_, objects_,
                            params_, camera_, planet_radius_);

    // カメラの滑らか移動制御
    spawnObject<EaseCamera>(fw_, objects_,
                            camera_);

    // 背景
    spawnObject<Bg>(fw_, objects_,
                    params_, camera_2d_, shader_holder_);
    spawnObject<Space>(fw_, objects_,
                       params_, camera_, model_holder_, shader_holder_);

    // 舞台となる惑星
    spawnObject<Planet>(fw_, objects_,
                        params_, model_holder_, shader_holder_, planet_radius_);
    
    // タイトル画面開始
    Signal::Params params;
    fw_.signal().sendMessage(Msg::START_TITLE, params);
  }

  ~GameProc() {
    DOUT << "~GameProc()" << std::endl;
    // シグナル解除
    signal_handle_.disconnect();
  }


  // 更新
  void update(const float delta_time) {
    {
      // 全オブジェクトへ更新指示
      Signal::Params params;
      params.insert(Signal::Params::value_type("delta_time", fix_framerate_ ? float(1.0 / 60) : delta_time));
      fw_.signal().sendMessage(Msg::UPDATE, params);
    }

    {
      // ゲーム内オブジェクトの情報収集
      Signal::Params params;
      fw_.signal().sendMessage(Msg::COLLECT_OBJECT_INFO, params);

      // ゲーム内オブジェクトの相互干渉
      fw_.signal().sendMessage(Msg::MUTUAL_INTERFERENCE, params);
    }
    
    // 有効でないオブジェクトを削除
    // TODO:このループを無くしたい
    size_t obj_num = objects_.size();
    while (obj_num > 0) {
      // 先頭のを取り出し、有効なら最後尾に追加する
      auto obj = objects_.front();
      objects_.pop_front();
      if (obj->isActive()) objects_.push_back(obj);
      
      --obj_num;
    }
    
    // カメラの逆行列を掛けると、カメラの行列を打ち消せる
    Quatf camera_inverse = camera_.rotate().inverse();

    // 光源はカメラの相対座標に設定
    for (auto& it : lights_) {
      it.second.pos() = camera_inverse * it.first;
    }

    // カメラの振動
    if (!pause_) {
      quake_camera_.update(delta_time);
    }
    
#ifdef _DEBUG
    // テキストのみモード
    char key = fw_.keyboard().getPushed();
    if (key == 'T') draw_text_only_ = !draw_text_only_;
    if (key == 't') draw_text_ = !draw_text_;
    
    if (key == 'G') gamecenter::deleteAchievements();
#endif
  }

  // 描画
  void draw() {
    // シェーダーの光源設定
    setupLights();

    // カメラ設定
    camera_.setup();
    quake_camera_.setup();
    setupProjectionMatrix();
    
    // 描画準備
    fw_.view().setupViewport();
    fw_.view().setupDraw();
    fw_.glState().depthTest(true);
    fw_.glState().cullFace(true);

    Signal::Params params;

    text_prims_.vtxes.clear();
    text_prims_.prims.clear();
    params.insert(Signal::Params::value_type("text_prims", &text_prims_));
    
    // 全オブジェクトへ描画指示
    fw_.signal().sendMessage(Msg::DRAW, params);

#ifdef _DEBUG
    if (draw_text_only_) {
      // テキスト描画のみモード:D
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    if (!draw_text_) return;
#endif
    
    // テキストを描画
    drawTextPrim(text_prims_);
  }


  // 一時停止
  void pause() {
    // 強制ポーズ
    Signal::Params params;
    fw_.signal().sendMessage(Msg::FORCE_PAUSE_GAME, params);
  }

  // 再開
  void resume() {
    DOUT << "GameProc::resume()" << std::endl;
    
    // 入力をフラッシュ
    Signal::Params params;
    fw_.signal().sendMessage(Msg::FLASH_TOUCH_INPUT, params);
    touch_widget_.flashTouch();
  }

  
  // ゲーム内の生成や破棄を一括して処理
  void message(const int msg, Signal::Params& arguments) {
    switch (msg) {
    case Msg::SPAWN_BASE:
      {
        auto obj = spawnObject<CubeBase>(fw_, objects_,
                                         params_, model_holder_, shader_holder_, shadow_);

        // 生成したオブジェクトにシグナル送信
        arguments.insert(Signal::Params::value_type("planet_radius", planet_radius_));
        obj->message(Msg::SET_SPAWN_INFO, arguments);
      }
      return;
      
    case Msg::SPAWN_PLAYER:
      {
        auto obj = spawnObject<CubePlayer>(fw_, objects_,
                                           params_, camera_, model_holder_, shader_holder_, shadow_);
      
        // 生成したオブジェクトにシグナル送信
        arguments.insert(Signal::Params::value_type("planet_radius", planet_radius_));
        obj->message(Msg::SET_SPAWN_INFO, arguments);
      }
      return;
      
    case Msg::SPAWN_ENEMY:
      {
        const std::string& name = boost::any_cast<std::string&>(arguments.at("name"));
        auto obj = spawnObject<CubeEnemy>(fw_, objects_,
                                          params_, name, cpu_factory_, model_holder_, shader_holder_, shadow_);

        // 生成したオブジェクトにシグナル送信
        arguments.insert(Signal::Params::value_type("planet_radius", planet_radius_));
        obj->message(Msg::SET_SPAWN_INFO, arguments);
      }
      return;

    case Msg::SPAWN_SIGNT:
      {
        auto obj = spawnObject<Signt>(fw_, objects_,
                                      params_, model_holder_, shader_holder_);

        // 生成したオブジェクトにシグナル送信
        arguments.insert(Signal::Params::value_type("planet_radius", planet_radius_));
        obj->message(Msg::SET_SPAWN_INFO, arguments);

        gamesound::play(fw_, "signt");
      }
      return;

    case Msg::SPAWN_ITEM:
      {
        auto obj = spawnObject<CubeItem>(fw_, objects_,
                                         params_, model_holder_, shader_holder_, shadow_);

        // 生成したオブジェクトにシグナル送信
        arguments.insert(Signal::Params::value_type("planet_radius", planet_radius_));
        obj->message(Msg::SET_SPAWN_INFO, arguments);

        gamesound::play(fw_, "item_entry");
      }
      return;

      
    case Msg::START_TITLE:
      {
        {
          // プレイヤー生成
          Signal::Params params;
          params.insert(Signal::Params::value_type("spawn_rotate", Quatf::Identity()));
          fw_.signal().sendMessage(Msg::SPAWN_PLAYER, params);
        }

        {
          // 開始時のカメラ演出
          Signal::Params params;
          EaseCamera::setupFromJson(params, game_params_.at("start_camera"));
          fw_.signal().sendMessage(Msg::START_EASE_CAMERA, params);
        }

        {
          // タイトル画面開始
          auto obj = spawnObject<TitleLogo>(fw_, objects_,
                                            params_, settings_.value(), font_, kana_font_, icon_font_, touch_widget_);
          Signal::Params params;
          setupTitleParams(params, false);
          obj->message(Msg::SET_SPAWN_INFO, params);
        }

        spawnObject<SkipTap>(fw_, objects_,
                             params_);
      }
      return;

    case Msg::START_TITLE_LOGO:
      {
        // タイトル画面開始(途中から)
        auto obj = spawnObject<TitleLogo>(fw_, objects_,
                                          params_, settings_.value(), font_, kana_font_, icon_font_, touch_widget_);
        Signal::Params params;
        setupTitleParams(params, true);
        obj->message(Msg::SET_SPAWN_INFO, params);

        spawnObject<SkipTap>(fw_, objects_,
                             params_);
      }
      return;


    case Msg::START_GAME:
      {
        // ゲーム本編開始
        spawnObject<GameStart>(fw_, objects_,
                               params_, settings_.value(), font_);
        auto obj = spawnObject<GameLogic>(fw_, objects_,
                                          params_, font_, number_font_, icon_font_, touch_widget_, lights_[2].second);

        Signal::Params params;
        params.insert(Signal::Params::value_type("best_score",
                                                 static_cast<int>(settings_.value().at("best_score").get<double>())));
        obj->message(Msg::SET_SPAWN_INFO, params);

        spawnObject<Generator>(fw_, objects_, params_);
      }
      return;

#if 0
    case Msg::START_GAMEMAIN:
      // ゲーム開始
      return;
#endif

    case Msg::START_GAME_RESULT:
      {
        // プレイ結果を収拾
        Signal::Params params;
        fw_.signal().sendMessage(Msg::GATHER_GAME_RESULT, params);

        // 結果を加工したりデーブデータに書き込んだりする
        handleGameResults(params);

        // 結果画面開始
        auto obj = spawnObject<GameResult>(fw_, objects_,
                                           params_, model_holder_, shader_holder_, font_, icon_font_, touch_widget_, camera_);
        
        // 生成したオブジェクトにシグナル送信
        obj->message(Msg::SET_SPAWN_INFO, params);

        // スキップ操作
        spawnObject<SkipTap>(fw_, objects_,
                             params_);
        spawnObject<AwaitTap>(fw_, objects_,
                              params_, font_, touch_widget_);
      }
      return;

    case Msg::DAMAGED_BASE:
      {
        // 基地がダメージを受けた
        quake_camera_.start(7.0f, 18.0f);        
      }
      return;
      
    case Msg::DESTROYED_BASE:
      {
        // 基地が破壊された
        quake_camera_.start(8.0f, 18.0f);
        // 警告音は中断
        gamesound::stop(fw_, "danger");

        spawnObject<GameOver>(fw_, objects_,
                              params_, settings_.value(), font_);

      }
      return;

      
    case Msg::TOUCHDOWN_PLANET:
      {
        // ジャンプアタック演出
        float power = boost::any_cast<float>(arguments.at("dist")) * 0.15f;
        float speed = boost::any_cast<float>(arguments.at("speed"));
        if (boost::any_cast<bool>(arguments.at("max_power"))) {
          // FIXME:アイテム効果による最大パワー攻撃がマジックナンバー…
          power = 6.5f;
          speed *= 0.85f;
        }
        
        quake_camera_.start(power, speed);
        
        arguments.insert(Signal::Params::value_type("planet_radius", planet_radius_));
#if 0
        {
          auto obj = spawnObject<AttackRange>(fw_, objects_,
                                              shader_holder_, camera_);
          obj->message(Msg::SET_SPAWN_INFO, arguments);
        }
#endif
        {
          auto obj = spawnObject<AttackEffect>(fw_, objects_,
                                               params_, shader_holder_, camera_);
          obj->message(Msg::SET_SPAWN_INFO, arguments);
        }
      }
      return;


    case Msg::PAUSE_GAME:
      // Pause
      spawnObject<PauseMenu>(fw_, objects_,
                             params_, font_, touch_widget_);
      pause_ = true;
      return;

    case Msg::RESUME_GAME:
      // Pause解除
      pause_ = false;
      return;

    case Msg::END_GAME:
      {
        quake_camera_.stop();
        pause_         = false;
        fix_framerate_ = false;

        // タイトル開始
        fw_.signal().sendMessage(Msg::START_TITLE, arguments);
      }
      return;

      
    case Msg::QUAKE_CAMERA:
      {
        quake_camera_.start(boost::any_cast<float>(arguments.at("power")),
                            boost::any_cast<float>(arguments.at("speed")));
      }
      return;


    case Msg::CHANGE_SOUND_SETTIGS:
      // サウンド設定変更
      soundSettings(arguments);
      return;


    case Msg::START_RECORDS:
      {
        // 記録画面起動
        spawnObject<Records>(fw_, objects_,
                             params_, model_holder_, shader_holder_, settings_.value(), font_, icon_font_, camera_, touch_widget_);
        // スキップ操作
        spawnObject<SkipTap>(fw_, objects_,
                             params_);
        spawnObject<AwaitTap>(fw_, objects_,
                              params_, font_, touch_widget_);

        // 実績
        gamecenter::achievement("NGS0004UIROU.records", 100.0);
      }
      return;

    case Msg::START_CREDITS:
      {
        // 記録画面起動
        spawnObject<Credits>(fw_, objects_,
                             params_, font_);
        
        // スキップ操作
        spawnObject<SkipTap>(fw_, objects_,
                             params_);
        spawnObject<AwaitTap>(fw_, objects_,
                              params_, font_, touch_widget_);

        // 実績
        gamecenter::achievement("NGS0004UIROU.credits", 100.0);
      }
      return;

      
#ifdef _DEBUG
    case Msg::TOGGLE_RECORD_MODE:
      {
        // 記録モードのトグル
        setRecordMode(!input_record_);
      }
      return;

    case Msg::TOGGLE_PLAYBACK_MODE:
      {
        // 入力再生モードのトグル
        setPlaybackMode(!input_playback_);
      }
      return;
#endif

    case Msg::FORCE_PLAYBACK_MODE:
      {
        // 再生モード
        setPlaybackMode(true);
      }
      return;

    case Msg::EXEC_DEMO_MODE:
      {
        // モードを引数に積む
        arguments.insert(Signal::Params::value_type("input_record", input_record_));
        arguments.insert(Signal::Params::value_type("input_playback", input_playback_));
        
        if (input_record_ || input_playback_) {
          // 乱数の初期化
          randomSetSeed();
        }
        
        // デモ開始
        spawnObject<DemoLogic>(fw_, objects_,
                               params_, font_);
        auto obj = spawnObject<GameLogic>(fw_, objects_,
                                          params_, font_, number_font_, icon_font_, touch_widget_, lights_[2].second);

        Signal::Params params;
        params.insert(Signal::Params::value_type("best_score",
                                                 static_cast<int>(settings_.value().at("best_score").get<double>())));
        params.insert(Signal::Params::value_type("demo_mode", true));
        params.insert(Signal::Params::value_type("input_record", input_record_));
        obj->message(Msg::SET_SPAWN_INFO, params);

        if (input_playback_) spawnObject<SkipTap>(fw_, objects_, params_);

        spawnObject<Generator>(fw_, objects_, params_);
      }
      return;

      
    default:
      return;
    }
  }

  
private:
  // 光源をパラメーターから読み込む
  void initLights(const picojson::value& params) {
    for (u_int i = 0; i < ELEMSOF(lights_); ++i) {
      const auto& light_param = params.get(i);

      auto& pos = lights_[i].first;
      auto& light = lights_[i].second;
      
      pos = vectFromJson<Vec3f>(light_param.at("pos"));

      light.pos()      = pos;
      light.diffuse()  = vectFromJson<Vec3f>(light_param.at("diffuse"));
      light.ambient()  = vectFromJson<Vec3f>(light_param.at("ambient"));
      light.specular() = vectFromJson<Vec3f>(light_param.at("specular"));
    }
  }
  
  // まとめて光源を設定
  void setupLights() {
    Light::setup(*shader_holder_.read("texture_light"), lights_[0].second);
    Light::setup(*shader_holder_.read("uirou_rank"), lights_[0].second);

    Light::setup(*shader_holder_.read("color_light_3"), lights_[0].second, lights_[1].second, lights_[2].second);
    Light::setup(*shader_holder_.read("uirou_white"), lights_[0].second, lights_[1].second, lights_[2].second);
    Light::setup(*shader_holder_.read("uirou_black"), lights_[0].second, lights_[1].second, lights_[2].second);
  }

  // 透視変換行列をまとめて設定
  void setupProjectionMatrix() {
    static const char* tbl[] = {
      "color",
      "color_light_3",
      "texture_light",
      "uirou_white",
      "uirou_black",
      "uirou_rank",
      "shadow"
    };

    // 現在の透視変換行列
    const Mat4f& projection = getProjectionMatrix();
      
    for (auto& name : tbl) {
      const auto& shader = *(shader_holder_.read(name));

      shader();
      glUniformMatrix4fv(shader.uniform("projectionMatrix"), 1, GL_FALSE, projection.data());
    }
  }
  
  // テキストをまとめて描画
  void drawTextPrim(const MatrixFont::PrimPack& text_prims) {
    if (text_prims.prims.empty()) return;

    fw_.glState().depthTest(false);
    fw_.glState().cullFace(false);

    // 2D用のカメラに切り替え
    camera_2d_.setup();

    // シェーター設定
    EasyShader& shader = *shader_holder_.read("font");
    shader();

    // 頂点データをVBOへ転送
    assert(text_prims.vtxes.size() < static_cast<size_t>(game_params_.at("font_vertex").get<double>()));
		glBindBuffer(GL_ARRAY_BUFFER, font_vbo_.handle());
		glBufferData(GL_ARRAY_BUFFER, sizeof(MatrixFont::Vtx) * text_prims.vtxes.size(), &text_prims.vtxes[0], GL_DYNAMIC_DRAW);
    
    // 頂点データ格納先を指示
    GLint position = shader.attrib("position");
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, sizeof(MatrixFont::Vtx), 0);

    // 表示用行列を生成
    Mat4f model_projection = getProjectionMatrix() * getModelMatrix();

    for (const auto& prim : text_prims.prims) {
      // 表示行列
      Mat4f m = model_projection * prim.matrix;
      glUniformMatrix4fv(shader.uniform("modelViewProjectionMatrix"), 1, GL_FALSE, m.data());

      // 表示色
      glUniform4f(shader.uniform("material_diffuse"), prim.color(0), prim.color(1), prim.color(2), prim.color(3));

      // 描画(頂点はVBOのを使う)
      glDrawArrays(GL_TRIANGLES, static_cast<GLsizei>(prim.index), static_cast<GLsizei>(prim.num));
    }

    // 後始末
    glDisableVertexAttribArray(position);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  // タイトル起動用のパラメータ生成
  void setupTitleParams(Signal::Params& params, const bool start_logo) {
    const picojson::value& settings = settings_.value();
    
    params.insert(Signal::Params::value_type("bgm_active", settings.at("bgm").get<bool>()));
    params.insert(Signal::Params::value_type("se_active", settings.at("se").get<bool>()));
    params.insert(Signal::Params::value_type("start_logo", start_logo));

    bool active_records = settings.at("number_of_played").get<double>() > 0;
    params.insert(Signal::Params::value_type("active_records", active_records));
  }
  
  // プレイ結果を加工したりセーブデータに格納したりする
  void handleGameResults(Signal::Params& params) {
    picojson::value& settings = settings_.value();

    // Best Scoreチェック
    int  score      = boost::any_cast<int>(params.at("score"));
    int  destroyed  = boost::any_cast<int>(params.at("destroyed_enemies"));
    int  best_score = int(settings.at("best_score").get<double>());
    bool new_best   = (score > best_score);
    int rank = minmax(static_cast<int>((std::sqrt(float(score)) - 20.0f) / 11.4f), 0, 7);
    
    // プレイ結果に追加
    best_score = std::max(score, best_score);
    params.insert(Signal::Params::value_type("best_score", best_score));
    params.insert(Signal::Params::value_type("new_best", new_best));
    params.insert(Signal::Params::value_type("rank", rank));

    // プレイデータも書き換える
    settings.at("best_score") = picojson::value(double(best_score));

    // 総合記録を更新
    modifyAddJson(settings, "number_of_played", 1);
    modifyAddJson(settings, "time_played", boost::any_cast<float>(params.at("play_time")));
    modifyAddJson(settings, "total_destroyed", destroyed);

    {
      // 取得したランクも記録
      const std::string& rank_name = params_.at("uirous").at("rank").get(rank).get<std::string>();
      settings.at("all_ranks").get<picojson::object>()[rank_name] = picojson::value(true);
      params.insert(Signal::Params::value_type("all_ranks", isGetAllRanks()));
    }
        
    int most_destroyed = int(settings.at("most_once_destroy").get<double>());
    most_destroyed = std::max(most_destroyed, boost::any_cast<int>(params.at("once_destroy_num")));
    settings.at("most_once_destroy") = picojson::value(double(most_destroyed));

    int most_combo_hit = int(settings.at("most_combo_hit").get<double>());
    most_combo_hit = std::max(most_combo_hit, boost::any_cast<int>(params.at("hit_combo_num")));
    settings.at("most_combo_hit") = picojson::value(double(most_combo_hit));

    // プレイデータを書き出し
    settings_.write();

    // 実績
    gamecenter::achievement("NGS0004UIROU.played", 100.0);

    int total_destroyed = static_cast<int>(settings.at("total_destroyed").get<double>());
    if (total_destroyed >= 10) {
      gamecenter::achievement("NGS0004UIROU.destroyed10", 100.0);
    }
    if (total_destroyed >= 100) {
      gamecenter::achievement("NGS0004UIROU.destroyed100", 100.0);
    }
    if (total_destroyed >= 1000) {
      gamecenter::achievement("NGS0004UIROU.destroyed1000", 100.0);
    }

    
    // SNS連携
    params.insert(Signal::Params::value_type("twitter", sns::canPost(sns::TWITTER)));
    params.insert(Signal::Params::value_type("facebook", sns::canPost(sns::FACEBOOK)));

    // GameCenterへ結果を送信
    gamecenter::sendResults(score, destroyed);
  }

  // サウンド設定変更
  void soundSettings(const Signal::Params& params) {
    bool bgm_active = boost::any_cast<bool>(params.at("bgm_active"));
    bool se_active  = boost::any_cast<bool>(params.at("se_active"));

    if (!bgm_active && gamesound::isActive(gamesound::BGM)) {
      gamesound::stop(fw_, gamesound::BGM);
    }
    if (!se_active && gamesound::isActive(gamesound::SE)) {
      gamesound::stop(fw_, gamesound::SE);
    }
    
    // 設定をフレームワークに反映
    gamesound::active(bgm_active, se_active);

    // 設定ファイルにも反映
    picojson::value& settings = settings_.value();

    settings.at("bgm") = picojson::value(bgm_active);
    settings.at("se")  = picojson::value(se_active);
    settings_.write();
  }

  
#ifdef _DEBUG
  // 記録モード変更
  void setRecordMode(const bool mode) {
    // 入力記録モードのトグル
    input_record_  = mode;
    fix_framerate_ = input_record_;

    // 記録と再生を同時に行わない
    if (input_record_) input_playback_ = false;
    DOUT << "TOGGLE_RECORD_MODE to " << input_record_ << std::endl;
  }
#endif

  // 再生モード変更
  void setPlaybackMode(const bool mode) {
    input_playback_ = mode;
    fix_framerate_  = input_playback_;

    // 記録と再生を同時に行わない
    if (input_playback_) input_record_ = false;
    DOUT << "TOGGLE_PLAYBACK_MODE to " << input_playback_ << std::endl;
  }

  
  // オブジェクトを生成してSignalに登録
  template <typename T>
  static std::shared_ptr<ObjBase> spawnObject(Framework& fw, std::deque<std::shared_ptr<ObjBase> >& objects) {
    std::shared_ptr<ObjBase> obj = std::make_shared<T>(fw);

    objects.push_back(obj);
    fw.signal().connect(obj);

    return obj;
  }
  
  template <typename T, typename T1>
  static std::shared_ptr<ObjBase> spawnObject(Framework& fw, std::deque<std::shared_ptr<ObjBase> >& objects,
                           T1& arg1) {
    std::shared_ptr<ObjBase> obj = std::make_shared<T>(fw, arg1);

    objects.push_back(obj);
    fw.signal().connect(obj);

    return obj;
  }

  template <typename T, typename T1, typename T2>
  static std::shared_ptr<ObjBase> spawnObject(Framework& fw, std::deque<std::shared_ptr<ObjBase> >& objects,
                           T1& arg1, T2& arg2) {
    std::shared_ptr<ObjBase> obj = std::make_shared<T>(fw, arg1, arg2);
    
    objects.push_back(obj);
    fw.signal().connect(obj);

    return obj;
  }

  template <typename T, typename T1, typename T2, typename T3>
  static std::shared_ptr<ObjBase> spawnObject(Framework& fw, std::deque<std::shared_ptr<ObjBase> >& objects,
                           T1& arg1, T2& arg2, T3& arg3) {
    std::shared_ptr<ObjBase> obj = std::make_shared<T>(fw, arg1, arg2, arg3);
    
    objects.push_back(obj);
    fw.signal().connect(obj);

    return obj;
  }

  template <typename T, typename T1, typename T2, typename T3, typename T4>
  static std::shared_ptr<ObjBase> spawnObject(Framework& fw, std::deque<std::shared_ptr<ObjBase> >& objects,
                           T1& arg1, T2& arg2, T3& arg3, T4& arg4) {
    std::shared_ptr<ObjBase> obj = std::make_shared<T>(fw, arg1, arg2, arg3, arg4);
    
    objects.push_back(obj);
    fw.signal().connect(obj);

    return obj;
  }

  template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5>
  static std::shared_ptr<ObjBase> spawnObject(Framework& fw, std::deque<std::shared_ptr<ObjBase> >& objects,
                           T1& arg1, T2& arg2, T3& arg3, T4& arg4, T5& arg5) {
    std::shared_ptr<ObjBase> obj = std::make_shared<T>(fw, arg1, arg2, arg3, arg4, arg5);
    
    objects.push_back(obj);
    fw.signal().connect(obj);

    return obj;
  }

  template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
  static std::shared_ptr<ObjBase> spawnObject(Framework& fw, std::deque<std::shared_ptr<ObjBase> >& objects,
                           T1& arg1, T2& arg2, T3& arg3, T4& arg4, T5& arg5, T6& arg6) {
    std::shared_ptr<ObjBase> obj = std::make_shared<T>(fw, arg1, arg2, arg3, arg4, arg5, arg6);
    
    objects.push_back(obj);
    fw.signal().connect(obj);

    return obj;
  }

  template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
  static std::shared_ptr<ObjBase> spawnObject(Framework& fw, std::deque<std::shared_ptr<ObjBase> >& objects,
                           T1& arg1, T2& arg2, T3& arg3, T4& arg4, T5& arg5, T6& arg6, T7& arg7) {
    std::shared_ptr<ObjBase> obj = std::make_shared<T>(fw, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    
    objects.push_back(obj);
    fw.signal().connect(obj);

    return obj;
  }

  template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
  static std::shared_ptr<ObjBase> spawnObject(Framework& fw, std::deque<std::shared_ptr<ObjBase> >& objects,
                           T1& arg1, T2& arg2, T3& arg3, T4& arg4, T5& arg5, T6& arg6, T7& arg7, T8& arg8) {
    std::shared_ptr<ObjBase> obj = std::make_shared<T>(fw, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    
    objects.push_back(obj);
    fw.signal().connect(obj);

    return obj;
  }


  // JSON読み込み(圧縮されたのがあればそれを優先して読む)
  static picojson::value readJsonFile(const std::string& load_path, const std::string& file) {
    std::string packed_file(changeFileExt(load_path + file, ".pack"));
    if (isFileExists(packed_file)) {
      // 圧縮されたのがあれば
      DOUT << file << " read from .pack" << std::endl;
      return readJsonZlib(packed_file);
    }
    else {
      // 通常
      return readJson(load_path + file);
    }
  }

  
  // 全ランク取得したか?
  bool isGetAllRanks() {
    // 用意したランク数
    size_t rank_num = params_.at("uirous").at("rank").get<picojson::array>().size();
    // 記録されたランク数と一致＝全ランク取得
    return rank_num == settings_.value().at("all_ranks").get<picojson::object>().size();
  }

};

}
