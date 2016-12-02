//
//  MuPfxPasswordView.m
//  MxPDF
//
//  Created by 曾亮 on 2016/12/2.
//  Copyright © 2016年 Artifex Software, Inc. All rights reserved.
//

#import "MuPfxPasswordView.h"
#import "common.h"
#include "mupdf/pdf.h"

@implementation MuPfxPasswordView {
	UILabel *_label;
	UILabel *_fpxfileLabel;
	UITextField *_textField;
	UIButton *buttonOk;
	UIButton *buttonCancel;
}

-(void) setSubViewBorderStyle : (UIView *)view {
	view.layer.cornerRadius = 4.0f;
	view.layer.borderColor = [[UIColor colorWithRed:0.7f green:0.7f blue:0.7f alpha:1.0f] CGColor];
	view.layer.borderWidth = 2.0f;
}

-(instancetype) initWithFrame: (CGRect)frame
{
	self = [super initWithFrame:frame];
	if(!self) return self;
	
	_label = [[UILabel alloc]initWithFrame:CGRectMake(10, 10, 40, 20)];
	_textField = [[UITextField alloc]initWithFrame:CGRectMake(50, 10, 200, 20)];
	buttonOk = [[UIButton alloc]initWithFrame:CGRectMake(50, 45, 50, 20)];
	buttonCancel = [[UIButton alloc]initWithFrame:CGRectMake(105, 45, 50, 20)];

	_label.text = @"证书密码:";
	_textField.secureTextEntry = YES;
	_textField.placeholder = @"input pfx file password";
	[buttonOk addTarget:self action:@selector(okTaped:) forControlEvents:UIControlEventTouchUpInside];
	[buttonCancel addTarget:self action:@selector(cancelTaped:) forControlEvents:UIControlEventTouchUpInside];
	
	[self setSubViewBorderStyle:_textField];
	[self setSubViewBorderStyle:buttonOk];
	[self setSubViewBorderStyle:buttonCancel];
	
	return self;
}

#if 0
z_device * device = z_openssl_new_device(ctx, RES_PATH"/user/zl.pfx", "111111");
fz_image * image = fz_new_image_from_file(ctx, RES_Image_file);
z_pdf_sign_appearance *app = z_pdf_new_image_sign_appearance(ctx, image,															(char*)"ntko(重庆软航科技有限公司)");
z_pdf_dosign(ctx, device, doc, pageno, rect, app);
z_pdf_drop_sign_appreance(ctx, app)
z_drop_device(ctx, device);
pdf_save_incremental_tofile(ctx, doc, savefile);
#endif

- (void) okTaped : (id)sender {
	// char *pfxfile = _textField.
	// z_device *device = z_openssl_new_device(ctx, _pfxfile get, <#char *pfxpassword#>)
}

- (void) cancelTaped : (id)sender {
	
}
@end
