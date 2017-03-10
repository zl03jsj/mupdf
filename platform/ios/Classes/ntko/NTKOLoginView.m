//
//  NTKOSignSvrSettings.m
//  MuPDF
//
//  Created by 曾亮 on 2017/2/22.
//  Copyright © 2017年 Artifex Software, Inc. All rights reserved.
//

#import "common.h"
#import "NTKOLoginView.h"
#import "MuDocumentController.h"
#import "NTKOEspFile.h"
#import "MuFileselectViewController.h"
#import "mupdf/z/ntko_svr.h"

@implementation NTKOLoginView
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
}

- (void) initNew:(MuDocumentController *)docVc VC:(UIViewController *)vc
{
	[super initNew:docVc VC:vc];
	
	self.delegate = docVc;
	
	_red_color = [[UIColor redColor]CGColor];
	_normal_color = nil;
}

- (BOOL) isInputTextValid :(UITextField*) input minlength:(int)len {
	if(nil==input) return NO;
	NSString *txt = [self inputText:input];
	return (nil!=txt&&txt.length>=len)?YES:NO;
}

- (BOOL) refreshInputStatus : (UITextField*)input minlength:(int)len errorHeader:(NSString *)header {
	BOOL isok = [self isInputTextValid:input minlength:len];
	CGColorRef color = _normal_color;
	NSString *message = nil;
	if(!isok) {
		color = _red_color;
		message = [NSString stringWithFormat:@"%@: can't be null, and need at last %d characters", header, len];
	}
	
	input.layer.borderColor = color;
	input.layer.borderWidth = 0.5f;
	input.layer.cornerRadius = 5.0f;
	
	if( nil!=message ) {
		NSString *tmp = _lb_message.text;
		if(nil==tmp || tmp.length==0)
			_lb_message.text = message;
		else
			_lb_message.text = [NSString stringWithFormat:@"%@\n%@", tmp, message];
	}
	return isok;
}

- (BOOL) refreshUIStatus {
	_lb_message.text = @"";
	BOOL isok,tmp;
	tmp = [self refreshInputStatus:_txt_serverurl minlength:3 errorHeader:@"server url"];
	isok = tmp;
	tmp = [self refreshInputStatus:_txt_username minlength:2 errorHeader:@"username"];
	if(isok) isok = tmp;
	tmp = [self refreshInputStatus:_txt_password minlength:6 errorHeader:@"password"];
	if(isok) isok = tmp;
	return isok;
}

- (void) initInputField {
	_txt_serverurl.text = [[NSUserDefaults standardUserDefaults] objectForKey:@"svrsign_url"];
	_txt_username.text = [[NSUserDefaults standardUserDefaults] objectForKey:@"svrsign_username"];
}

- (void) setViewBorder:(UIView*)view {
	view.layer.borderWidth = 0.5f;
	view.layer.borderColor = _normal_color;
	view.layer.cornerRadius = 3.0f;
}

- (void)awakeFromNib {
	_normal_color = [[UIColor grayColor]CGColor];
	_bt_login.clipsToBounds=YES;
	
	[self setViewBorder:_bt_login];
	[self setViewBorder:_lb_message];
	[self setViewBorder:_txt_serverurl];
	[self setViewBorder:_txt_username];
	[self setViewBorder:_txt_password];

	[self initInputField];
	
	[super awakeFromNib];
}

- (void) layoutSubviews {
}

- (NSString*) inputText:(UITextField*)input {
	return [input.text stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
}

- (BOOL) doLogin {
	if(!_ssCtx || !_ssCtx->username || !_ssCtx->password)
		return NO;
	fz_try(ctx) {
		_ssCtx->logined = ntko_http_login(ctx, _ssCtx->username, _ssCtx->password, &_ssCtx->status, &_ssCtx->svrinfo, &_ssCtx->rights);
	}
	fz_catch(ctx) {
		_ssCtx->logined = false;
		fz_rethrow(ctx);
	}
	return _ssCtx->logined?YES:NO;
}

- (void)afterDoLogin:(BOOL)loginOk {
	dispatch_async(dispatch_get_main_queue(), ^(void) {
		[_indcator stopAnimating];
		if(loginOk) {
			_lb_message.text = @"login success, getting esp list...";
 			[_indcator startAnimating];
			if(_delegate) {
				[_delegate OnLoginOk];
			}
		}
		else {
			_bt_login.enabled = YES;
			if(_ssCtx->status.code!=0 && _ssCtx->status.fail_reason) {
				_lb_message.text = [NSString stringWithFormat:@"login failed:%@", [NSString stringWithUTF8String:_ssCtx->status.fail_reason]];
			}
		}
	});
}

- (void)onLoginExeption {
	dispatch_async(dispatch_get_main_queue(), ^(void) {
		_lb_message.text = [NSString stringWithFormat:@"login exeption:%s", fz_caught_message(ctx) ];
	});
}

- (IBAction)onLoginTap:(id)sender {
	if(!_ssCtx) return;
	/* first check is input validate*/
	_lb_message.hidden = NO;
	_bt_statusimage.hidden = NO;
	
	BOOL isok = [self refreshUIStatus];
	if(!isok) return;
	
	/* store new settings */
	[[NSUserDefaults standardUserDefaults] setObject: _txt_serverurl.text forKey: @"svrsign_url"];
	[[NSUserDefaults standardUserDefaults] setObject: _txt_username.text forKey: @"svrsign_username"];
	
	if(_ssCtx->username) fz_free(ctx, _ssCtx->username);
	if(_ssCtx->password) fz_free(ctx, _ssCtx->password);
	if(_ssCtx->svrinfo.settingurl) fz_free(ctx, _ssCtx->svrinfo.settingurl);
	
	_ssCtx->username = fz_strdup(ctx, [self inputText:_txt_username].UTF8String);
	_ssCtx->password = fz_strdup(ctx, [self inputText:_txt_password].UTF8String);
	_ssCtx->svrinfo.settingurl = fz_strdup(ctx, [self inputText:_txt_serverurl].UTF8String);
	
	[_indcator startAnimating];
	_bt_login.enabled = NO;
	
	dispatch_async(queue, ^{
		BOOL isok = NO;
		fz_try(ctx)
			isok = [self doLogin];
		fz_catch(ctx) {
			[self onLoginExeption];
			isok = NO;
		}
		
		[self afterDoLogin:isok];
	});
}

/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect {
    // Drawing code
}
*/
- (void)dealloc {
	if(_block) Block_release(_block);
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
