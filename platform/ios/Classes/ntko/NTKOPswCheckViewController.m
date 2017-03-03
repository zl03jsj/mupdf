//
//  NTKOPswCheckViewController.m
//  MxPDF
//
//  Created by 曾亮 on 2016/12/3.
//  Copyright © 2016年 Artifex Software, Inc. All rights reserved.
//

#import "NTKOPswCheckViewController.h"
#import "NTKOTableDs.h"
#import "common.h"

@interface NTKOPswCheckViewController ()

@property (retain, nonatomic) IBOutlet UILabel *pfxfilename;
@property (retain, nonatomic) IBOutlet UITextField *pfxfilepassword;
@property (retain, nonatomic) IBOutlet UILabel *message;

@end

@implementation NTKOPswCheckViewController {
	int _remainingTimes;
	id<NTKOPswCheckViewDelegate> _delegate;
	PswCheckBlock _block;
	id<NTKOTableDs> _nfile;
	IBOutlet UIButton *_btOkay;
	
	UIActivityIndicatorView* _checkingView;
}

- (instancetype)initWithNtkoFile:(id<NTKOTableDs>)ntkofile Delegate:(id<NTKOPswCheckViewDelegate>)delegate CheckBlock:(PswCheckBlock)block
{
	self = [super init];
	if(self) {
		_remainingTimes = 4;
		_nfile = [ntkofile retain];
		_delegate = delegate;
		_block = Block_copy(block);
		_checkingView = nil;
	}
	return self;
}

- (void) startAnimation {
	if(!_checkingView)
		_checkingView = [[UIActivityIndicatorView alloc]initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhiteLarge];
	
	dispatch_async(dispatch_get_main_queue(), ^(void) {
		[_checkingView startAnimating];
		[self.view addSubview: _checkingView];
	});
}

- (void) stopAnimation {
	if(!_checkingView)
		_checkingView = [[UIActivityIndicatorView alloc]initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhiteLarge];
	
	dispatch_async(dispatch_get_main_queue(), ^(void) {
		if(_checkingView.isAnimating) {
			[_checkingView stopAnimating];
			[self.view willRemoveSubview:_checkingView];
		}
	});
}
//- (instancetype)initWithFilename: (NSString*)pfxfilename {
//	self = [super init];
//	NSFileManager *fileman = [NSFileManager defaultManager];
//	BOOL isdir;
//	if( [fileman fileExistsAtPath:pfxfilename isDirectory:&isdir] && !isdir) {
//		_deviceokblock = nil;
//		_pfxfile = [pfxfilename retain];
//		_remainingTimes = 4;
//		return self;
//	}
//	return nil;
//}

//+ (void)verifyPfxPassword:(UIViewController*)parentVc pfxfile:(NSString *)file pfxPswCheckViewDelegate:(id<MuPfxPswViewDelegate>)delegate deviceCreateOkBlock:(void(^)(z_device *))deviceokblock {
//	NTKOPswCheckViewController *pfxpasswordView = [[NTKOPswCheckViewController alloc]initWithFilename:file];
//	if(!pfxpasswordView)
//		return;
//
//	pfxpasswordView.delegate = delegate;
//	pfxpasswordView.modalPresentationStyle = UIModalPresentationFormSheet;
//	pfxpasswordView.modalTransitionStyle = UIModalTransitionStyleCrossDissolve;
//	
////	CGSize size = parentVc.view.bounds.size;
////	parentVc.preferredContentSize = CGSizeMake(size.width*3/4, size.height*3/4);
//	[parentVc presentViewController:pfxpasswordView animated:NO completion:^{
////		UIDeviceOrientation orient = [[UIDevice currentDevice] orientation];
////		if(orient==UIDeviceOrientationPortrait ||
////		   orient==UIDeviceOrientationPortraitUpsideDown) {}else{}
//		CGSize size = parentVc.view.bounds.size;
//		CGPoint center = parentVc.view.center;
//		pfxpasswordView.view.superview.frame = CGRectMake(0, 0, size.width*3/4, size.height*3/4);
//		pfxpasswordView.view.superview.center = CGPointMake(center.x,  fz_max(0.0f, center.y-50));
//		// pfxpasswordView.view.layer.cornerRadius = 5;
//		pfxpasswordView.view.layer.masksToBounds = YES;
//	}];
//	
//	[pfxpasswordView release];
//}

- (IBAction)onCloseTaped:(id)sender {
	[self.navigationController popToRootViewControllerAnimated:YES];
}

//- (void) close {
//	// close all
//	UIViewController *vc = self.presentingViewController;
//	if ( !vc.presentingViewController ) {
//		[self dismissViewControllerAnimated:YES completion:nil];
//		return;
//	}
//	while (vc.presentingViewController)
//		vc = vc.presentingViewController;
//	[vc dismissViewControllerAnimated:YES completion:nil];
//}

//- (IBAction)onOkTaped:(id)sender {
//	const char *pfxfile = [_pfxfile cStringUsingEncoding:NSUTF8StringEncoding];
//	const char *pfxpassword = [_pfxfilepassword.text cStringUsingEncoding:NSUTF8StringEncoding];
//	
//	z_device *device = NULL;
//	fz_try(ctx) {
//		device = z_openssl_new_device(ctx, (char*)pfxfile, (char*)pfxpassword);
//		if( device ) {
//			if(_deviceokblock) _deviceokblock(device);
//			if(_delegate) [_delegate deviceCreateOk:device];
//			z_drop_device(ctx, device);
//			device = nil;
//			[self close];
//		}
//	}
//	fz_catch(ctx) {
//		if(0==(--_remainingTimes)) {
//			if(_delegate) [_delegate deviceCreateFailed];
//			[self close];
//		}
//		_message.text = [NSString stringWithFormat:@"error: you still have %d time chances of inputing. \nerror detail:%@",
//						 _remainingTimes, [NSString stringWithUTF8String:ctx->error->message],nil];
//	}
//}

- (IBAction)onOkTaped:(id)sender {
	if(_delegate) {
		dispatch_async(queue, ^(void) {
			[self startAnimation];
			BOOL isok = [_delegate onPswCheck:self Ds:_nfile Password:_pfxfilepassword.text];
			[self stopAnimation];
			if(!isok)
				[self onPswFailed];
		});
	}
}

- (void)onPswFailed {
	dispatch_async(dispatch_get_main_queue(), ^(void) {
		_remainingTimes--;
		if(0==_remainingTimes) {
			if(_delegate)
				[_delegate onPswViewClose:_remainingTimes];
			[self.navigationController popViewControllerAnimated:YES];
		}
		else
			_message.text = [NSString stringWithFormat:@"open file failed, still have %d chances.",_remainingTimes];
	});
}

- (void)viewDidLoad {
    [super viewDidLoad];
	self.navigationItem.title = [NSString stringWithFormat:@"%@ Password", _nfile.title];
	_pfxfilename.text = _nfile.title;
	
	_btOkay.layer.borderColor = [[UIColor grayColor] CGColor];
	_btOkay.layer.borderWidth = 0.5f;
	_btOkay.layer.cornerRadius = 4;
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

- (void)dealloc {
	if(_block) Block_release(_block);
	[_pfxfilename release];
	[_pfxfilepassword release];
	[(id)_nfile release];
	[_message release];
	[_btOkay release];
	[super dealloc];
}
@end
