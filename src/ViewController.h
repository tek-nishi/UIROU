//
//  ViewController.h
//  game05
//
//  Created by Nishiyama Nobuyuki on 2012/08/19.
//  Copyright (c) 2012年 Nishiyama Nobuyuki. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>
#ifndef VER_LITE
#import <GameKit/GameKit.h>
#endif
#ifdef VER_LITE
#import <iAD/iAD.h>
#endif


#if defined (_DEBUG) && defined (VER_LITE)
// LITE版 Debug
@interface ViewController : GLKViewController<UITextFieldDelegate, ADBannerViewDelegate, UIAlertViewDelegate>
#elif defined (_DEBUG)
// FULL版 Debug
@interface ViewController : GLKViewController<GKLeaderboardViewControllerDelegate, UIAlertViewDelegate, UITextFieldDelegate>
#elif defined (VER_LITE)
// LITE版 Release
@interface ViewController : GLKViewController<ADBannerViewDelegate, UIAlertViewDelegate>
#else
// FULL版 Release
@interface ViewController : GLKViewController<GKLeaderboardViewControllerDelegate, UIAlertViewDelegate>
#endif


@property (nonatomic, strong) IBOutlet UITextField* debug_text;
@property (nonatomic, assign) int alert_mode;
@property (nonatomic, strong) NSString* alert_url;

#ifndef VER_LITE

// 実績のローカルキャッシュ
@property(nonatomic, strong) NSMutableDictionary* achievements_dictionary;
@property(nonatomic, strong) NSString* achievements_cache_file;

- (void)saveAchievements;

#endif

#ifdef VER_LITE

// iAd
@property (nonatomic, strong) ADBannerView* ad_banner;

#endif




- (void)pause;
- (void)resume;

#ifdef _DEBUG
- (IBAction)textExit:(id)sender;
#endif

@end
