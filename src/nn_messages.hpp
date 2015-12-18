
#pragma once

//
// アプリ内のメッセージ定義
//

#include "co_collision.hpp"
#include "nn_objBase.hpp"


namespace ngs {

struct Msg {
  // FIXME:置き場がないための苦肉の策
  struct BaseInfo {
    ObjBase* obj;

    u_int hash;
    Vec3f pos;
    Vec3f angle;
    Quatf rotate;
    float radius;
    float scale;

    int hp;
    int hp_max;
  };

  struct PlayerInfo {
    ObjBase* obj;

    u_int hash;
    bool  collision;
    Vec3f pos;
    Vec3f angle;
    float radius;

    bool jumping;
  };
  
  struct EnemyInfo {
    ObjBase* obj;

    u_int hash;
    bool  collision;
    Vec3f pos;
    Vec3f angle;
    float radius;

    bool jumping;
  };

  struct ItemInfo {
    ObjBase* obj;

    u_int hash;
    Vec3f pos;
    float radius;
  };

  enum {
    // 更新
    UPDATE,
    // 描画
    DRAW,

    // タッチ処理をフラッシュ
    FLASH_TOUCH_INPUT,
    // 惑星タッチを１回だけスキップ
    MANIPULATE_ONCE_SKIP,

    // ジャンプアタック開始
    START_JUMPATTACK,
    
    // 基地生成
    SPAWN_BASE,
    // プレイヤー生成
    SPAWN_PLAYER,
    // CubeEnemy生成
    SPAWN_ENEMY,
    // CubeItem生成
    SPAWN_ITEM,

    // 攻撃目標オブジェクト生成
    SPAWN_SIGNT,
    // 照準回復
    RECOVER_SIGNT,

    // 生成時の位置や向きを設定
    SET_SPAWN_INFO,

    // プレイヤー攻撃→着地
    TOUCHDOWN_PLANET,

    // ゲーム内オブジェクトの収集
    COLLECT_OBJECT_INFO,
    // 相互干渉
    MUTUAL_INTERFERENCE,
    
    // 敵と基地の接触判定
    CHECK_HIT_BASE,
    
    // 基地がダメージを受けた
    DAMAGED_BASE,
    // 基地が破壊された
    DESTROYED_BASE,

    // 敵を破壊した
    DESTROYED_ENEMY,
    // 破壊出来なかったが攻撃は当たった
    ATTACK_HIT_ENEMY,
    // 消滅時にアイテムを生む
    ENEMY_SPAWN_ITEM,

    // アイテムを拾った
    PICK_UP_ITEM,

    // アイテム効果
    ITEM_MAX_POWER,
    ITEM_ENEMY_STIFF,
    ITEM_BASE_IMMORTAL,
    ITEM_SCORE_MULTIPLY,
    ITEM_MAX_RANGE,

    // レベルアップ
    GAME_LEVELUP,
    
    // タイトル開始
    START_TITLE,
    // タイトル開始(途中から)
    START_TITLE_LOGO,
    
    // ゲーム本編開始
    START_GAME,

    // ゲーム開始
    START_GAMEMAIN,

    // 結果画面開始
    START_GAME_RESULT,
    
    // ゲーム本編終了
    END_GAME,

    // ゲーム終了演出完了
    FINISH_GAME_OVER,
    
    // プレイ結果の収拾
    GATHER_GAME_RESULT,
    
    // ゲーム一時停止
    PAUSE_GAME,
    // ゲーム再開
    RESUME_GAME,
    // ゲーム中断
    ABORT_GAME,

    // アプリがバックグラウンドになったので強制ポーズ
    FORCE_PAUSE_GAME,
    
    // カメラの滑らか移動開始
    START_EASE_CAMERA,
    // カメラの滑らか移動強制終了
    TO_END_EASE_CAMERA,
    // カメラの滑らか移動終了
    END_EASE_CAMERA,
    // カメラの振動
    QUAKE_CAMERA,
    
    // サウンド環境の変更
    CHANGE_SOUND_SETTIGS,

    // 記録画面起動
    START_RECORDS,
    // 著作表示
    START_CREDITS,

    // 「画面スキップ」操作
    FINISH_SKIP_TAP,
    // 中断(スキップされなかった)
    ABORT_SKIP_TAP,
    
    // 「次へ進む」開始
    START_AWAIT_TAP,
    //  「次の画面へ」操作
    FINISH_AWAIT_TAP,

    // 記録モードのトグル
    TOGGLE_RECORD_MODE,
    TOGGLE_PLAYBACK_MODE,
    // 再生モード
    FORCE_PLAYBACK_MODE,

    PLAYER_RECORD_INFO,
    PLAYER_PLAYBACK_INFO,

    // 再生データ終了
    PLAYBACK_FIN,

    // DEMOモードで動かす
    EXEC_DEMO_MODE,
    
  };
};

}
