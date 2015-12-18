// 
// GameCenter関連
//

#include "co_defines.hpp"
#include <iostream>
#import <GameKit/GameKit.h>
#include "ViewController.h"
#include "gamecenter.h"


#if defined (USE_GAMECENTER)

namespace ngs {
namespace gamecenter {

  // FIXME:汚い…
static bool use_ = false;
static ViewController* view_controller_ = nil;


// GameCenterが有効ならtrue
bool canUse() {
  return use_;
}


// 実績のローカルキャッシュを更新
static void loadAchievements() {
  [GKAchievement loadAchievementsWithCompletionHandler:^(NSArray* achievements, NSError* error) {
      if (error == nil) {
				NSLOG(@"Achievements OK!");

        // 読み込めたら、ローカルキャッシュを再構築
        view_controller_.achievements_dictionary = [[[NSMutableDictionary alloc] init] autorelease];

        for (GKAchievement* achievement in achievements) {
          [view_controller_.achievements_dictionary
              setObject:achievement forKey:achievement.identifier];
        }
        
        // 実機のキャッシュを書き出しておく
        [view_controller_ saveAchievements];
      }
      else {
        // FIXME:ローカルとGameCenterでのつじつま合わせ
				NSLOG(@"Achievements read error:%@", [error localizedDescription]);
      }
    }];
}


// GameCenterへログイン
void login(ViewController* view_controller) {
	NSLOG(@"Login GameCenter");
  view_controller_ = view_controller;
  
	GKLocalPlayer* localPlayer = [GKLocalPlayer localPlayer];
	[localPlayer authenticateWithCompletionHandler:^(NSError* error) {
			if (error != nil) {
				NSLOG(@"Login error:%@", [error localizedDescription]);
			}

			if (localPlayer.isAuthenticated) {
        // GameCenterに無事ログイン出来た
        use_ = true;
        
				NSLOG(@"Login OK!");
        loadAchievements();
			}
		}];
}


// スコア送信
static void sendScore(const int score, NSString* category) {
  GKScore* scoreReporter = [[[GKScore alloc] initWithCategory:category] autorelease];
  NSInteger value = score;
  scoreReporter.value = value;

  [scoreReporter reportScoreWithCompletionHandler:^(NSError* error) {
      if (error != nil) {
        NSLOG(@"Sending score error:%@", [error localizedDescription]);
      }
      else {
        NSLOG(@"Sending score OK!");
      }
    }];
}


void sendResults(const int score, const int destroyed) {
  if (!canUse()) {
    NSLOG(@"gamecenter::sendResults: GameCenter is not active.");
    return;
  }
  
	NSLOG(@"Send results to GameCenter");

  if (score > 0) sendScore(score, @"NGS0004UIROU.normal");
  if (destroyed > 0) sendScore(destroyed, @"NGS0004UIROU.most.destroyed");
}


// 実績
static GKAchievement* getAchievementForIdentifier(NSString* identifier) {
  GKAchievement* achievement = [view_controller_.achievements_dictionary
                                   objectForKey:identifier];
  if (achievement == nil) {
    achievement = [[[GKAchievement alloc] initWithIdentifier:identifier] autorelease];
    [view_controller_.achievements_dictionary
        setObject:achievement forKey:achievement.identifier];
  }
  return achievement;
}


void achievement(const std::string& name, const double percent, const bool demo) {
  // DEMOモードの場合は無視
  if (demo) {
    NSLOG(@"demo mode!!");
    return;
  }
  
  if (!canUse()) {
    NSLOG(@"gamecenter::achievement: GameCenter is not active.");
    return;
  }

  NSString* identifier = [[[NSString alloc] initWithCString:name.c_str() encoding:NSUTF8StringEncoding] autorelease];
  GKAchievement* achievement = getAchievementForIdentifier(identifier);
  if (achievement) {
    double prev_percent = achievement.percentComplete;
    if (percent != prev_percent) {
      achievement.percentComplete = percent;
      achievement.showsCompletionBanner = YES;
      [achievement reportAchievementWithCompletionHandler:^(NSError* error) {
          if (error != nil) {
            NSLOG(@"Error in reporting achievements:%@", [error localizedDescription]);
          }
        }];

      // 実機のキャッシュを書き出しておく
      [view_controller_ saveAchievements];
    }
    else {
      DOUT << "achievement:" << name << " already completed." << std::endl;
    }
  }
}


#ifdef _DEBUG

// すべての実績を削除
void deleteAchievements() {
  NSLOG(@"gamecenter::deleteAchievements()");
  
  // 実績のローカルキャッシュをクリア
  view_controller_.achievements_dictionary = [[[NSMutableDictionary alloc] init] autorelease];
  // 実機のキャッシュを書き出しておく
  [view_controller_ saveAchievements];
  
  [GKAchievement resetAchievementsWithCompletionHandler:^(NSError* error) {
      if (error != nil) {
          NSLOG(@"Error in resetting achievements:%@", [error localizedDescription]);
      }
      else {
          NSLOG(@"deleteAchievements OK!");
      }
    }];
}

#endif


static void show(NSString* category) {
	GKLeaderboardViewController* leaderboardController = [[[GKLeaderboardViewController alloc] init] autorelease];
  
	if (leaderboardController != nil) {
    [view_controller_ pause];
    
		leaderboardController.category = category;
		leaderboardController.leaderboardDelegate = view_controller_;
		[view_controller_ presentModalViewController: leaderboardController animated: YES];
	}
}

// GameCenter表示
void showGamecenter() {
  show(nil);
}

//  Leaderboard表示
void showLeaderboard() {
  show(@"NGS0004UIROU.normal");
}

}
}

#endif
