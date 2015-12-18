//
// Twitter投稿(iOS5用)
// 

#import "co_defines.hpp"
#import <string>
#import <Twitter/Twitter.h>
#import "Twitter.h"


namespace ngs {
namespace twitter {

// FIXME:汚い…
static ViewController* view_controller_ = nil;
static bool can_post_ = false;


// 初期化(iOSが古いのもチェック)
bool init(ViewController* view_controller) {
  view_controller_ = view_controller;
  
	Class clazz = NSClassFromString(@"TWTweetComposeViewController");
	if (clazz) {
		NSLOG(@"Tweet OK!");
		can_post_ = true;
	}
  return can_post_;
}

// ツイート可能か返す
bool canPost() {
	return can_post_ && [TWTweetComposeViewController canSendTweet];
}

// ツイート実行
void post(const std::string& text, UIImage* image) {
  if (!canPost()) return;

  NSLOG(@"twitter::post");

  // postするテキストを生成
	NSString* str = [[[NSString alloc] initWithCString:text.c_str() encoding:NSUTF8StringEncoding] autorelease];
  
  TWTweetComposeViewController* viewController = [[[TWTweetComposeViewController alloc] init] autorelease];

  // 完了処理をBlockで定義
  void (^completion) (TWTweetComposeViewControllerResult result) = ^(TWTweetComposeViewControllerResult result) {
    switch (result) {
    case TWTweetComposeViewControllerResultCancelled:
    case TWTweetComposeViewControllerResultDone:
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
