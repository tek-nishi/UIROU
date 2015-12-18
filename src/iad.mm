//
// iAD関連
//

#include "co_defines.hpp"
#import "iad.h"


#ifdef VER_LITE

namespace ngs {
namespace iad {

// 生成
ADBannerView* createBanner(UIInterfaceOrientation orientation) {
  NSLOG(@"iad::createBanner");

	ADBannerView* view = [[[ADBannerView alloc] initWithFrame:CGRectMake(0,0,0,0)] autorelease];
	view.currentContentSizeIdentifier =
    UIInterfaceOrientationIsPortrait(orientation) ? ADBannerContentSizeIdentifierPortrait : ADBannerContentSizeIdentifierLandscape;

	return view;
}

// バナーのサイズを返す
CGSize getBannerSize(UIInterfaceOrientation orientation) {
	NSString* identifier =
    UIInterfaceOrientationIsPortrait(orientation) ? ADBannerContentSizeIdentifierPortrait : ADBannerContentSizeIdentifierLandscape;

	return [ADBannerView sizeFromBannerContentSizeIdentifier:identifier];
}

// バナーに画面の縦横を伝える
void fixupView(ADBannerView* view, UIInterfaceOrientation orientation) {
	view.currentContentSizeIdentifier =
    UIInterfaceOrientationIsPortrait(orientation) ? ADBannerContentSizeIdentifierPortrait : ADBannerContentSizeIdentifierLandscape;
}

}
}

#endif
