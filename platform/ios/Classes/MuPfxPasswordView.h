//
//  MuPfxPasswordView.h
//  MxPDF
//
//  Created by 曾亮 on 2016/12/3.
//  Copyright © 2016年 Artifex Software, Inc. All rights reserved.
//

#import <UIKit/UIKit.h>
#include "mupdf/pdf.h"

typedef void(^SignDeviceOkBlock)(z_device*);

@protocol MuPfxPswViewDelegate
- (BOOL)deviceCreateOk:(z_device *)device;
- (void)deviceCreateFailed;
@end

@interface MuPfxPasswordView : UIViewController
@property (copy, nonatomic) void(^deviceokblock)(z_device*);
@property (assign, nonatomic) id<MuPfxPswViewDelegate> delegate;
- (instancetype)initWithFilename: (NSString*)pfxfilename;
+ (void)verifyPfxPassword:(UIViewController*)parentVc pfxfile:(NSString *)file pfxPswCheckViewDelegate:(id<MuPfxPswViewDelegate>)delegate deviceCreateOkBlock:(void(^)(z_device *))deviceOkBlock;
@end
