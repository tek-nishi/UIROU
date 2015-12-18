// 
// 評価ダイアログ
//

#include "co_defines.hpp"
#import "ViewController.h"
#include "rating.h"


namespace ngs {
namespace rating {

const NSInteger rate_frequency = 8;
  
static ViewController* view_controller_;


// 初期化
void init(ViewController* view_controller) {
  view_controller_ = view_controller;
}

// ダイアログ表示
void popup() {
  NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];

  // 実行済みなら処理しない
  if ([defaults boolForKey:@"rate_exec"]) return;

  // 10回ごとに実行する
  NSInteger rate_num = [defaults integerForKey:@"rate_num"];
  [defaults setInteger:rate_num + 1 forKey:@"rate_num"];

  NSLOG(@"rating:rate_num:%ld", long(rate_num));
  if ((rate_num % rate_frequency) != (rate_frequency - 1)) return;

  // アプリの実行を止める
  [view_controller_ pause];

  // アプリの評価へのリンク
  view_controller_.alert_mode = 0;

#ifdef VER_LITE
  NSString* app_id = @"697740169";
#else
  NSString* app_id = @"697739733";
#endif

#if 1

  // FIXME:iOS7では評価ページに直接リンクできないので、とりあえずアプリのページを表示
  view_controller_.alert_url = [NSString stringWithFormat:@"itms-apps://itunes.apple.com/en/app/id%@", app_id];
  
#else
  
  view_controller_.alert_url = [NSString stringWithFormat:@"itms-apps://itunes.apple.com/WebObjects/MZStore.woa/wa/viewContentsUserReviews?type=Purple+Software&id=%@", app_id];

#endif
  
  UIAlertView* alert = [[UIAlertView alloc]
                           initWithTitle: NSLocalizedString(@"rate_title", nil)
                           message: NSLocalizedString(@"rate_text", nil)
                           delegate: view_controller_
                           cancelButtonTitle: NSLocalizedString(@"rate_cancel", nil)
                           otherButtonTitles: NSLocalizedString(@"rate_now", nil), NSLocalizedString(@"rate_later", nil),
                           nil];
  [alert show];
  [alert release];
}

// 初期化
void reset() {
  NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
  [defaults setBool:NO forKey:@"rate_exec"];
}

}
}
