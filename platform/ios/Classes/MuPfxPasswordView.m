//
//  MuPfxPasswordView.m
//  MxPDF
//
//  Created by 曾亮 on 2016/12/3.
//  Copyright © 2016年 Artifex Software, Inc. All rights reserved.
//

#import "MuPfxPasswordView.h"
#import "common.h"

@interface MuPfxPasswordView ()

@property (retain, nonatomic) IBOutlet UILabel *pfxfilename;
@property (retain, nonatomic) IBOutlet UITextField *pfxfilepassword;
@property (retain, nonatomic) IBOutlet UILabel *message;

@end

@implementation MuPfxPasswordView {
	int _remainingTimes;
	id<MuPfxPswViewDelegate> _delegate;
	NSString *_pfxfile;
}

- (instancetype)initWithFilename: (NSString*)pfxfilename {
	self = [super init];
	NSFileManager *fileman = [NSFileManager defaultManager];
	BOOL isdir;
	if( [fileman fileExistsAtPath:pfxfilename isDirectory:&isdir] && !isdir) {
		_deviceokblock = nil;
		_pfxfile = [pfxfilename retain];
		_remainingTimes = 4;
		return self;
	}
	return nil;
}

+ (void)verifyPfxPassword:(UIViewController*)parentVc pfxfile:(NSString *)file pfxPswCheckViewDelegate:(id<MuPfxPswViewDelegate>)delegate deviceCreateOkBlock:(void(^)(z_device *))deviceokblock {
	MuPfxPasswordView *pfxpasswordView = [[MuPfxPasswordView alloc]initWithFilename:file];
	if(!pfxpasswordView)
		return;

	pfxpasswordView.delegate = delegate;
	pfxpasswordView.modalPresentationStyle = UIModalPresentationFormSheet;
	pfxpasswordView.modalTransitionStyle = UIModalTransitionStyleCrossDissolve;
	
//	CGSize size = parentVc.view.bounds.size;
//	parentVc.preferredContentSize = CGSizeMake(size.width*3/4, size.height*3/4);
	[parentVc presentViewController:pfxpasswordView animated:NO completion:^{
//		UIDeviceOrientation orient = [[UIDevice currentDevice] orientation];
//		if(orient==UIDeviceOrientationPortrait ||
//		   orient==UIDeviceOrientationPortraitUpsideDown) {}else{}
		CGSize size = parentVc.view.bounds.size;
		CGPoint center = parentVc.view.center;
		pfxpasswordView.view.superview.frame = CGRectMake(0, 0, size.width*3/4, size.height*3/4);
		pfxpasswordView.view.superview.center = CGPointMake(center.x,  fz_max(0.0f, center.y-50));
		pfxpasswordView.view.layer.cornerRadius = 5;
		pfxpasswordView.view.layer.masksToBounds = YES;
	}];
	
	[pfxpasswordView release];
}

- (IBAction)onCloseTaped:(id)sender {
	[self dismissViewControllerAnimated:YES completion:nil];

#if defined(Mupfxpassword_use_block_callback)
	if(_cancelblock) {
		_cancelblock(_pfxfilepassword.text);
	}
#endif
}

- (IBAction)onOkTaped:(id)sender {
	const char *pfxfile = [_pfxfile cStringUsingEncoding:NSUTF8StringEncoding];
	const char *pfxpassword = [_pfxfilepassword.text cStringUsingEncoding:NSUTF8StringEncoding];
	
	z_device *device = NULL;
	fz_try(ctx) {
		device = z_openssl_new_device(ctx, (char*)pfxfile, (char*)pfxpassword);
		_remainingTimes--;
		if(0==_remainingTimes) {
			[self dismissViewControllerAnimated:YES completion:nil];
		}
		if( device ) {
			if(_deviceokblock) _deviceokblock(device);
			if(_delegate) [_delegate deviceCreateOk:device];
			z_drop_device(ctx, device);
			device = nil;
			
			// close all
			UIViewController *vc = self.presentingViewController;
			if ( !vc.presentingViewController ) {
				[self dismissViewControllerAnimated:YES completion:nil];
				return;
			}
			while (vc.presentingViewController)
				vc = vc.presentingViewController;
			[vc dismissViewControllerAnimated:YES completion:nil];
		}
	}
	fz_catch(ctx) {
		_message.text = [NSString stringWithFormat:@"error: you still have %d time chances of inputing. \nerror detail:%@",_remainingTimes, [NSString stringWithUTF8String:ctx->error->message],nil];
	}
}

- (void)viewDidLoad {
    [super viewDidLoad];
	_pfxfilename.text = [_pfxfile lastPathComponent];
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
	if(_deviceokblock) Block_release(_deviceokblock);
	[_pfxfilename release];
	[_pfxfilepassword release];
	[_pfxfile release];
	[_message release];
	[super dealloc];
}
@end
