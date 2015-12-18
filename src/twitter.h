//
// Twitter関連(iOS5用)
//

#pragma once

#if (TARGET_OS_IPHONE)

#include <string>
#import "ViewController.h"


namespace ngs {
namespace twitter {

bool init(ViewController* view_controller);
bool canPost();
void post(const std::string& text, UIImage* image);

}
}

#endif
