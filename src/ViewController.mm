//
// アプリ実行画面
// 

// TIPS:GLKitでES1.1のヘッダを読むのを防ぐ
#define ES1_GL_H_GUARD
#define ES1_GLEXT_H_GUARD

#import <AudioToolbox/AudioToolbox.h>
#import "ViewController.h"
#import "sns.h"
#import "iad.h"
#import "gamecenter.h"
#import "rating.h"
#import "advertisement.h"
#include "co_execGLKit.hpp"


// AudioSettionカテゴリ設定
static void setupAudioCategory() {
  // 他のアプリのサウンドとミックスする
	UInt32 cat = kAudioSessionCategory_AmbientSound;
	AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(UInt32), &cat);
}

// Audio一時停止
static void suspendAudio() {
	NSLOG(@"suspendAudio");

	AudioSessionSetActive(NO);
  if (ngs::exec::app_instance) {
    ngs::exec::app_instance->audio().suspend();
  }
}

// Audio再開
static void processAudio() {
	NSLOG(@"processAudio");
		
	setupAudioCategory();
	AudioSessionSetActive(YES);
  if (ngs::exec::app_instance) {
    ngs::exec::app_instance->audio().process();
  }
	// iOS5.1だとAudioSessionInitialize()のコールバックでこの設定するとエラーになる
}


@interface ViewController ()
{
  BOOL _view_loaded;
  BOOL _observing;
  BOOL _paused;
}

@end


@implementation ViewController


#ifdef VER_LITE

- (void)setupBannerLayout:(UIInterfaceOrientation)interfaceOrientation
{
	// CGRect view_bounds = self.view.bounds;
	// CGSize banner_size = ngs::iad::getBannerSize(interfaceOrientation);

  CGRect ad_bounds = _ad_banner.frame;
  ad_bounds.origin.x = 0.0f;
  ad_bounds.origin.y = 0.0f;
// ad_bounds.origin.y = view_bounds.size.height - banner_size.height;
  _ad_banner.frame = ad_bounds;

  ngs::iad::fixupView(self.ad_banner, interfaceOrientation);

  // バナーのサイズを変更したらアプリ内のオフセットを変更
  ad_bounds = _ad_banner.frame;
  ngs::exec::app_instance->view().topOffset(ad_bounds.size.height * ngs::exec::screen_scale);
}

#endif


// アプリ起動時の初期化
- (void)viewDidLoad
{
	NSLOG(@"viewDidLoad");

	[super viewDidLoad];
  
  if (!_view_loaded) {
    _view_loaded = YES;

    // 他のアプリの音の再生を許可
    AudioSessionInitialize(NULL, NULL, NULL, NULL);
    setupAudioCategory();
    
    // アプリ環境の生成
    GLKView* view = (GLKView*)self.view;
    ngs::exec::initialize(view);
    
#ifndef VER_LITE
    // 実績のローカルキャッシュ
    NSString* file_path = ngs::Os::osGetDocumentPath();
    self.achievements_cache_file = [file_path stringByAppendingString:@"/achievements.save"];
    if ([[NSFileManager defaultManager] fileExistsAtPath:_achievements_cache_file]) {
      NSLOG(@"achievements cache read:%@", _achievements_cache_file);
      self.achievements_dictionary = [NSKeyedUnarchiver unarchiveObjectWithFile:_achievements_cache_file];
    }
    else {
      NSLOG(@"achievements cache create");
      _achievements_dictionary = [[NSMutableDictionary alloc] init];
    }
#endif

#ifdef VER_LITE
    {
      // iAdバナー生成
      UIInterfaceOrientation interfaceOrientation = self.interfaceOrientation;
      ADBannerView* ad_banner = ngs::iad::createBanner(interfaceOrientation);
      self.ad_banner = ad_banner;

      // 初期設定
      ad_banner.autoresizingMask = UIViewAutoresizingNone;
      ad_banner.hidden = YES;
      ad_banner.delegate = self;

      [self.view addSubview:ad_banner];
      [self setupBannerLayout:interfaceOrientation];
    }
#endif

    // SNS連携
    ngs::sns::init(self, view);

    // GameCenter
    ngs::gamecenter::login(self);

    // 評価ダイアログ
    ngs::rating::init(self);

    // 宣伝ダイアログ
    ngs::advertisement::init(self);

    // FPS設定
    self.preferredFramesPerSecond = ngs::App::FRAMERATE;

#ifdef _DEBUG
    _debug_text.hidden = NO;
#endif

    // FIXME:画面更新前に初期化を行うための苦肉の策
    ngs::exec::update();
  }
}

// 破棄
- (void)dealloc 
{
  ngs::exec::destroy();

#ifndef VER_LITE
  // 実績のキャッシュ
  [_achievements_cache_file release];
  [_achievements_dictionary release];
#endif

#ifdef VER_LITE
  // iAd
	self.ad_banner.delegate = nil;
  [_ad_banner release];
#endif

  [_debug_text release];
  [_alert_url release];
	[super dealloc];
}

#ifndef VER_LITE

// 実績キャッシュの書き出し
- (void)saveAchievements
{
  [NSKeyedArchiver archiveRootObject:_achievements_dictionary toFile:_achievements_cache_file];
}

#endif


- (void)didReceiveMemoryWarning
{
	NSLOG(@"didReceiveMemoryWarning");

	[super didReceiveMemoryWarning];

#if 0
  // TIPS:バックグラウンド状態では呼ばれないので、なにもしなくてよい
  if ([self isViewLoaded] && ([[self view] window] == nil)) {
    NSLOG(@"destroy view");
    
    GLKView* view = (GLKView*)self.view;
    if ([EAGLContext currentContext] == view.context) {
      NSLOG(@"EAGLContext:setCurrentContext:nil");
      [EAGLContext setCurrentContext:nil];
    }
    self.view = nil;
  }
#endif
}


- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
	return YES;
}

#if defined (VER_LITE)

-(void)willAnimateRotationToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation duration:(NSTimeInterval)duration
{
	NSLOG(@"willAnimateRotationToInterfaceOrientation:%ld", long(interfaceOrientation));

  // iAdバナーのレイアウト変更
  [self setupBannerLayout:interfaceOrientation];

	[super willAnimateRotationToInterfaceOrientation:interfaceOrientation duration:duration];
}

#endif


#ifdef _DEBUG

// UITextFieldが変更されたら呼ばれる
- (BOOL)textField:(UITextField *)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string
{
	NSLOG(@"textField:shouldChangeCharactersInRange:replacementString:");
//  NSLog(@"%@", string);

  if ([string length] > 0) {
    unichar chara = [string characterAtIndex:0];
    ngs::exec::keyInput(chara);
  }
  return YES;
}

// DoneやEnterが押されたらキーボードをしまう
- (IBAction)textExit:(id)sender
{
  _debug_text.text = @"";
  [_debug_text resignFirstResponder];
}

// キーボードONで画面下部のテキストフィールドを移動
- (void)keyboardWillShow:(NSNotification *)notification
{
	NSLOG(@"keyboardWillShow:");

  // キーボード出現時のパラメーターを引っ張りだす
  NSDictionary *dic = [notification userInfo];
  CGRect keyboard_rect = [[dic objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
  NSTimeInterval duration = [[dic objectForKey:UIKeyboardAnimationDurationUserInfoKey] doubleValue];
  UIViewAnimationCurve curve = static_cast<UIViewAnimationCurve>([[dic objectForKey:UIKeyboardAnimationCurveUserInfoKey] integerValue]);

  // デバイスの向き
  BOOL landscape = UIInterfaceOrientationIsLandscape([[UIApplication sharedApplication] statusBarOrientation]);
  
  // アニメーション開始
  void (^animations)(void);
  animations = ^(void) {
    CGRect frame = _debug_text.frame;
    // TIPS:デバイスの向きでオフセットが違う
    frame.origin.y -= landscape ? keyboard_rect.size.width : keyboard_rect.size.height;
    _debug_text.frame = frame;
  };
  [UIView animateWithDuration:duration delay:0.0f options:curve animations:animations completion:nil];
}

// キーボードONで画面下部のテキストフィールドを元に戻す
- (void)keyboardWillHide:(NSNotification *)notification
{
	NSLOG(@"keyboardWillHide:");

  NSDictionary *dic = [notification userInfo];
  CGRect keyboard_rect = [[dic objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
  NSTimeInterval duration = [[dic objectForKey:UIKeyboardAnimationDurationUserInfoKey] doubleValue];
  UIViewAnimationCurve curve = static_cast<UIViewAnimationCurve>([[dic objectForKey:UIKeyboardAnimationCurveUserInfoKey] integerValue]);

  // デバイスの向き
  BOOL landscape = UIInterfaceOrientationIsLandscape([[UIApplication sharedApplication] statusBarOrientation]);

  void (^animations)(void);
  animations = ^(void) {
    CGRect frame = _debug_text.frame;
    // TIPS:デバイスの向きでオフセットが違う
    frame.origin.y += landscape ? keyboard_rect.size.width : keyboard_rect.size.height;
    _debug_text.frame = frame;
  };
  [UIView animateWithDuration:duration delay:0.0f options:curve animations:animations completion:nil];
}

#endif


// アプリ画面が最前列になった時に呼び出される
- (void)viewWillAppear:(BOOL)animated
{
	NSLOG(@"viewWillAppear:");
  [super viewWillAppear:animated];

#if defined (VER_LITE)
  [self setupBannerLayout:self.interfaceOrientation];
#endif

#ifdef _DEBUG
  if (!_observing) {
    _observing = YES;

    // キーボードON/OFFの通知を受け取る
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(keyboardWillShow:) name:UIKeyboardWillShowNotification object:nil];
    [center addObserver:self selector:@selector(keyboardWillHide:) name:UIKeyboardWillHideNotification object:nil];
  }
#endif
}

// アプリ画面が隠れる時に呼び出される
- (void)viewWillDisappear:(BOOL)animated
{
	NSLOG(@"viewWillDisappear:");
  [super viewWillDisappear:animated];

#ifdef _DEBUG
  if (_observing) {
    _observing = NO;

    // キーボードON/OFFの通知を解除
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    [center removeObserver:self name:UIKeyboardWillShowNotification object:nil];
    [center removeObserver:self name:UIKeyboardWillHideNotification object:nil];
  }
#endif
}


#if 0

- (void)viewDidDisappear:(BOOL)animated
{
	NSLOG(@"viewDidDisappear:");
  [super viewDidDisappear:animated];
}

- (void)viewDidAppear:(BOOL)animated
{
	NSLOG(@"viewDidDisappear:");
  [super viewDidAppear:animated];
}

#endif


// タッチ開始イベント
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
  if (_paused) return;
  ngs::exec::createTouchStartInfo(touches, self.view);
}

// タッチ位置移動イベント
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
  if (_paused) return;
  ngs::exec::createTouchMoveInfo(touches, self.view);
}

// タッチ終了イベント
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
  if (_paused) return;
  ngs::exec::createTouchEndInfo(touches, self.view);
}

// 着信などでタッチイベントを中断
- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
  if (_paused) return;
  ngs::exec::createTouchEndInfo(touches, self.view);
}


// アプリ一時停止
- (void)pause
{
  NSLOG(@"app pause");
  
  _paused = true;
  ngs::exec::pause();
  suspendAudio();
}

// アプリ再開
- (void)resume
{
  NSLOG(@"app resume");

  processAudio();
  ngs::exec::resume();
  _paused = false;
}


// 更新
- (void)update
{
  ngs::exec::update();
}

// 表示
- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
  ngs::exec::draw(int(view.drawableWidth), int(view.drawableHeight));
}


#if defined (USE_GAMECENTER)

//リーダーボードで完了を押した時に呼ばれる
- (void)leaderboardViewControllerDidFinish:(GKLeaderboardViewController *)viewController
{
  [self dismissViewControllerAnimated:YES completion:nil];
  [self resume];
}

#endif


#ifdef VER_LITE

// iAd読み込み完了
- (void)bannerViewDidLoadAd:(ADBannerView *)banner
{
  DOUT << "bannerViewDidLoadAd:" << std::endl;
	banner.hidden = NO;

  // TIPS:バナーのレイアウトを再設定
  // iOS6のiPadでは、横画面でアプリを起動すると
  // 縦画面のレイアウトでiAdが動いてしまう
  [self setupBannerLayout:self.interfaceOrientation];
}
 
- (void)bannerView:(ADBannerView *)banner didFailToReceiveAdWithError:(NSError *)error
{
  NSLOG(@"bannerView:didFailToReceiveAdWithError:%@", [error localizedDescription]);
	banner.hidden = YES;
}

#endif


- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
  DOUT << "alertView:" << _alert_mode << " " << buttonIndex << std::endl;

  // buttonIndexの0番がキャンセルボタンな為、キャンセルボタンが無い場合は処理の流れが変わる
  if (alertView.cancelButtonIndex == -1) buttonIndex += 1;
  
  NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
  switch (_alert_mode) {
  case 0:
    {
      // 評価ダイアログ
      switch (buttonIndex) {
      case 0:
        // 評価しない
        NSLOG(@"Rating:No Thanks");

        [defaults setBool:YES forKey:@"rate_exec"];
        break;

      case 1:
        // OK
        NSLOG(@"Rating:OK");

        [[UIApplication sharedApplication] openURL:[NSURL URLWithString:_alert_url]];
        [defaults setBool:YES forKey:@"rate_exec"];
        break;

      case 2:
        // 後で
        NSLOG(@"Rating:Remind me later");
        break;
      }
    }
    break;

  case 1:
    {
      // 宣伝ダイアログ
      switch (buttonIndex) {
      case 0:
        // 宣伝見ない
        NSLOG(@"Advertisement:No Thanks");

        [defaults setBool:YES forKey:@"advertisement_exec"];
        break;

      case 1:
        // OK
        NSLOG(@"Advertisement:OK");

        [[UIApplication sharedApplication] openURL:[NSURL URLWithString:_alert_url]];
        [defaults setBool:YES forKey:@"advertisement_exec"];
        break;

      case 2:
        // 後で
        NSLOG(@"Advertisement:Remind me later");
        break;
      }
    }
    break;
  }
  
  [self resume];
}


@end
