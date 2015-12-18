//
// SNS連携
//

#pragma once

#include <string>

namespace ngs {
namespace sns {
  
enum Type {
  TWITTER,
  FACEBOOK
};

}
}

#if (TARGET_OS_IPHONE)

#import "ViewController.h"

namespace ngs {
namespace sns {

void init(ViewController* view_controller, GLKView* view);
bool canPost(const Type type);
void post(const Type type, const std::string& text);
std::string postText(const Type type);

}
}

#else

namespace ngs {
namespace sns {

bool canPost(const Type type) { return false; }
void post(const Type type, const std::string& text) {}
std::string postText(const Type type) { return ""; }
  
}
}

#endif
