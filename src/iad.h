//
// iAD関連
//
#pragma once

#if (TARGET_OS_IPHONE) && defined (VER_LITE)

#import <iAD/iAD.h>

namespace ngs {
namespace iad {

ADBannerView* createBanner(UIInterfaceOrientation orientation);
CGSize getBannerSize(UIInterfaceOrientation orientation);
void fixupView(ADBannerView* view, UIInterfaceOrientation orientation);

}
}

#else

namespace ngs {
namespace iad {

}
}

#endif
