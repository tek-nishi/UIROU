// 
// 宣伝ダイアログ
//

#include "co_defines.hpp"
#import "ViewController.h"
#include "advertisement.h"


namespace ngs {
namespace advertisement {


#ifdef VER_LITE
const NSInteger ad_frequency = 6;
#else
const NSInteger ad_frequency = 12;
#endif
  
static ViewController* view_controller_;


// 初期化
void init(ViewController* view_controller) {
  view_controller_ = view_controller;
}

// ダイアログ表示
void popup() {
  NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];

#if !defined (VER_LITE)
  // 実行済みなら処理しない(LITE版はずっと出る)
  if ([defaults boolForKey:@"advertisement_exec"]) return;
#endif

  // 12回ごとに実行する(LITE版は6回)
  NSInteger advertisement_num = [defaults integerForKey:@"advertisement_num"];
  [defaults setInteger:advertisement_num + 1 forKey:@"advertisement_num"];

  NSLOG(@"advertisement:advertisement_num:%ld", long(advertisement_num));
  if ((advertisement_num % ad_frequency) != (ad_frequency - 1)) return;
  
  // アプリの実行を止める
  [view_controller_ pause];

  // NGSへのリンク
  view_controller_.alert_mode = 1;
  view_controller_.alert_url = @"itms-apps://itunes.com/apps/newgamestyle";
  
  UIAlertView* alert = [[UIAlertView alloc]
                           initWithTitle: NSLocalizedString(@"advertisement_title", nil)
                           message: NSLocalizedString(@"advertisement_text", nil)
                           delegate: view_controller_
#ifdef VER_LITE
                           cancelButtonTitle: nil
#else
                           cancelButtonTitle: NSLocalizedString(@"advertisement_cancel", nil)
#endif
                           otherButtonTitles: NSLocalizedString(@"advertisement_now", nil), NSLocalizedString(@"advertisement_later", nil),
                           nil];
  [alert show];
  [alert release];
}

// 初期化
void reset() {
  NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
  [defaults setBool:NO forKey:@"advertisement_exec"];
}

}
}
