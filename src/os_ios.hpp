
#pragma once

//
// OS依存処理(iOS版)
//

#if (TARGET_OS_IPHONE)

#import <UIKit/UIKit.h>
#include <iostream>
#include <string>
#include <boost/noncopyable.hpp>


namespace ngs {

class Os : private boost::noncopyable {
  std::string loadPath_;
  std::string savePath_;

public:
  Os() {
    DOUT << "Os()" << std::endl;

    NSString* resourcePath = [[NSBundle mainBundle] resourcePath];
    loadPath_ = [resourcePath UTF8String] + std::string("/res/");

    NSString* documentPath = osGetDocumentPath();
    savePath_ = [documentPath UTF8String] + std::string("/");
  }

  ~Os() {
    DOUT << "~Os()" << std::endl;
  }

  const std::string& loadPath() const { return loadPath_; }
  const std::string& savePath() const { return savePath_; }


  // サンドボックスのパスを取得
  static NSString* osGetDocumentPath() {
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    return [paths objectAtIndex:0];
  }
  
};

}

#endif
