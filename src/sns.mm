//
// SNS連携
//

#include "co_defines.hpp"
#include <cassert>
#import "sns.h"
#import "twitter.h"
#import "social.h"


namespace ngs {
namespace sns {

enum Framework {
  NONE,
  SOCIAL_FW,
  TWITTER_FW
};

static Framework framework_type_ = NONE;
static GLKView* game_view_ = nil;

// 画面のスナップショット
// TIPS:GLKView:snapshotではAlphaが有効なので、それを削除したのを返す
static UIImage* snapshot() {
  UIImage* image = [game_view_ snapshot];
  
  // スナップショットのアルファ値を捨てるため、UIImageを再構築してから使う
  UIGraphicsBeginImageContextWithOptions(image.size, YES, 0.0f);

  // オフスクリーンレンダリング
  CGContextRef ctx = UIGraphicsGetCurrentContext();
  CGRect area = CGRectMake(0.0f, 0.0f, image.size.width, image.size.height);

  // 上下をひっくり返す
  CGContextScaleCTM(ctx, 1.0f, -1.0f);
  CGContextTranslateCTM(ctx, 0.0f, -area.size.height);
  
  CGContextDrawImage(ctx, area, image.CGImage);

  // UIImageを取得
  UIImage* image_non_alpha = UIGraphicsGetImageFromCurrentImageContext();

  UIGraphicsEndImageContext();

  return image_non_alpha;
}


// 初期化
void init(ViewController* view_controller, GLKView* view) {
  game_view_ = view;
  
  // iOSのバージョンで使えるframeworkが違う
  if (social::init(view_controller)) {
    // iOS6以降
		NSLOG(@"Social.framework OK!!");
    framework_type_ = SOCIAL_FW;
    return;
  }
  
  if (twitter::init(view_controller)) {
    // iOS5以降
		NSLOG(@"Twitter.framework OK!!");
    framework_type_ = TWITTER_FW;
    return;
  }
  
  NSLOG(@"No SNS framework available.");
}

// Postできるか返す
bool canPost(const Type type) {
  switch (framework_type_) {
  case NONE:
    return false;

  case SOCIAL_FW:
    switch (type) {
    case TWITTER:
      return social::canPost(social::TWITTER);

    case FACEBOOK:
      return social::canPost(social::FACEBOOK);

    default:
      assert(0);
      return false;
    }

  case TWITTER_FW:
    switch (type) {
    case TWITTER:
      return twitter::canPost();

    case FACEBOOK:
      return false;

    default:
      assert(0);
      return false;
    }

  default:
    assert(0);
    return false;
  }
}

// Postする
void post(const Type type, const std::string& text) {
  switch (framework_type_) {
  case NONE:
    return;

  case SOCIAL_FW:
    switch (type) {
    case TWITTER:
      social::post(social::TWITTER, text, snapshot());
      return;

    case FACEBOOK:
      social::post(social::FACEBOOK, text, snapshot());
      return;

    default:
      assert(0);
      return;
    }

  case TWITTER_FW:
    switch (type) {
    case TWITTER:
      twitter::post(text, snapshot());
      return;

    case FACEBOOK:
      assert(0);
      return;

    default:
      assert(0);
      return;
    }

  default:
    assert(0);
    return;
  }
}

// 投稿用の文字列(ローカライズ済み)を取得
std::string postText(const Type type) {
  NSString* text;
  
  switch (type) {
  case TWITTER:
    text = NSLocalizedString(@"twitter", nil);
    break;

  case FACEBOOK:
    text = NSLocalizedString(@"facebook", nil);
    break;

  default:
    assert(0);
    text = @"";
    break;
  }

  return [text UTF8String];
}


}
}
