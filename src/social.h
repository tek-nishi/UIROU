//
// SNS関連(iOS6用)
//

#pragma once

#if (TARGET_OS_IPHONE)

#include <string>
#import "ViewController.h"


namespace ngs {
namespace social {

enum Type {
  TWITTER,
  FACEBOOK
};

bool init(ViewController* view_controller);
bool canPost(const Type type);
void post(const Type type, const std::string& text, UIImage* image);

}
}

#endif
