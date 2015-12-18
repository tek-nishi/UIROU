//
// SNS投稿(iOS6用)
// 

#import "co_defines.hpp"
#import <string>
#import <cassert>
#import <Social/Social.h>
#import "Social.h"


namespace ngs {
namespace social {

// FIXME:汚い…
static ViewController* view_controller_ = nil;
static bool can_post_ = false;


// 初期化(iOSが古いのもチェック)
bool init(ViewController* view_controller) {
  view_controller_ = view_controller;
  
	Class clazz = NSClassFromString(@"SLComposeViewController");
	if (clazz) {
		NSLOG(@"Tweet OK!");
		can_post_ = true;
	}
  return can_post_;
}

// enumから文字列に変換
static NSString* const typeFromEnum(const Type type) {
  switch (type) {
  case TWITTER:
    return SLServiceTypeTwitter;

  case FACEBOOK:
    return SLServiceTypeFacebook;

  default:
    assert(0);
    return nil;
  }
}

// ツイート可能か返す
bool canPost(const Type type) {
	return can_post_ && [SLComposeViewController isAvailableForServiceType:typeFromEnum(type)];
}

// ツイート実行
void post(const Type type, const std::string& text, UIImage* image) {
  if (!canPost(type)) return;

  NSLOG(@"social::post");

  // postするテキストを生成
	NSString* str = [[[NSString alloc] initWithCString:text.c_str() encoding:NSUTF8StringEncoding] autorelease];

  // autoreleaseでなくてよさげ
  SLComposeViewController* viewController = [SLComposeViewController composeViewControllerForServiceType:typeFromEnum(type)];

  // 完了処理をBlockで定義
  void (^completion) (SLComposeViewControllerResult result) = ^(SLComposeViewControllerResult result) {
    switch (result) {
    case SLComposeViewControllerResultCancelled:
    case SLComposeViewControllerResultDone:
      // TIPS:自分でdismissしないといけない
      [viewController dismissViewControllerAnimated:YES completion:nil];
      // アプリの実行を再開
      [view_controller_ resume];
      NSLOG(@"completion");
    }
  };
  [viewController setCompletionHandler:completion];

  [viewController setInitialText:str];
  [viewController addImage:image];
  [viewController addURL:[NSURL URLWithString:@"https://itunes.apple.com/us/app/uirou-lite/id697740169?mt=8"]];

  // アプリの実行を一時停止
  [view_controller_ pause];
  // 投稿ダイアログを表示
  [view_controller_ presentViewController:viewController animated:YES completion:NULL];
}

}
}
