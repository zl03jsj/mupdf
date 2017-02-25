//
//  NTKOSignSvrSettings.m
//  MuPDF
//
//  Created by 曾亮 on 2017/2/22.
//  Copyright © 2017年 Artifex Software, Inc. All rights reserved.
//

#import "common.h"
#import "NTKOSignSvrSettings.h"
#import "MuDocumentController.h"

@interface NTKOSignSvrSettings()
@property (nonatomic, copy) NSString *testprop1;
@end

@implementation NTKOSignSvrSettings
{
	IBOutlet UITextField *_txt_serverurl;
	IBOutlet UITextField *_txt_username;
	IBOutlet UITextField *_txt_password;
	IBOutlet UIButton *_bt_login;
	IBOutlet UIActivityIndicatorView *_indcator;
	IBOutlet UIButton *_bt_statusimage;
	IBOutlet UILabel *_lb_message;
	
	CGColorRef _normal_color;
	CGColorRef _red_color;
	
	MuDocumentController *_target;
}

- (instancetype) initWithTarget:(id)target {
	self = [super initWithTarget:target];
	if(self) {
		_red_color = [[UIColor redColor]CGColor];
		_normal_color = nil;
	}
	return self;
}

- (BOOL) isInputTextValid :(UITextField*) input minlength:(int)len {
	if(nil==input) return NO;
	NSString *txt = [input.text stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
	
	BOOL isValid = YES;
	if(nil==txt || txt.length<=len)
		isValid = NO;
	
	return isValid;
}

- (BOOL) refreshInputStatus : (UITextField*)input minlength:(int)len errorHeader:(NSString *)header {
	BOOL isok = [self isInputTextValid:_txt_serverurl minlength:len];
	CGColorRef color = _normal_color;
	NSString *message = nil;
	if(!isok) {
		color = _red_color;
		message = [NSString stringWithFormat:@"%@: can't be null, and need at last %d characters", header, len];
	}
	input.layer.borderColor = color;
	if(nil!=message) {
		NSString *tmp = _lb_message.text;
		if(nil==tmp || tmp.length==0)
			_lb_message.text = message;
		else
			_lb_message.text = [NSString stringWithFormat:@"%@\n%@", tmp, message];
	}
	return isok;
}

- (BOOL) refreshUIStatus {
	BOOL isok = YES;
	_lb_message.text = @"";
	isok = [self refreshInputStatus:_txt_serverurl minlength:3 errorHeader:@"server url"];
	isok = isok?YES: [self refreshInputStatus:_txt_username minlength:2 errorHeader:@"username"];
	isok = isok?YES: [self refreshInputStatus:_txt_password minlength:6 errorHeader:@"password"];
	return isok;
}

- (void) initInputField {
	_txt_serverurl.text = [[NSUserDefaults standardUserDefaults] objectForKey:@"svrsign_url"];
	_txt_username.text = [[NSUserDefaults standardUserDefaults] objectForKey:@"svrsign_username"];
}
- (void) layoutSubviews {
	_normal_color = _txt_serverurl.layer.borderColor;
	
	_bt_login.clipsToBounds=YES;
	
	CALayer *layer = _bt_login.layer;
	CGColorRef color = [[UIColor colorWithRed:0.6f green:0.6f blue:1.0f alpha:1.0f] CGColor];
	
	layer.borderColor = color;
	layer.borderWidth = 1.0f;
	layer.cornerRadius = 5;
	
	[self initInputField];
}

- (IBAction)onLoginTap:(id)sender {
	BOOL isok = [self refreshUIStatus];
	
	[_indcator startAnimating];
	[[NSUserDefaults standardUserDefaults] setObject: _txt_serverurl.text forKey: @"svrsign_url"];
	[[NSUserDefaults standardUserDefaults] setObject: _txt_username.text forKey: @"svrsign_username"];
	
	if(!isok) return;
}

/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect {
    // Drawing code
}
*/
- (void)dealloc {
	[_txt_serverurl release];
	[_txt_username release];
	[_txt_password release];
	[_bt_login release];
	[_indcator release];
	[_bt_statusimage release];
	[_lb_message release];
	[super dealloc];
}
@end
