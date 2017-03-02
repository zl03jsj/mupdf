//
//  NTKOPswCheckViewController.h
//  MxPDF
//
//  Created by 曾亮 on 2016/12/3.
//  Copyright © 2016年 Artifex Software, Inc. All rights reserved.
//

#import <UIKit/UIKit.h>
#include "mupdf/pdf.h"

@protocol NTKOTableDs;
@class NTKOPswCheckViewController;

typedef void(^PswCheckBlock)(z_device*);

@protocol NTKOPswCheckViewDelegate
- (BOOL) onPswCheck:(NTKOPswCheckViewController*)checkVc Ds:(id<NTKOTableDs>)ds Password:(NSString*)psw;
@end

@interface NTKOPswCheckViewController : UIViewController
- (instancetype)initWithNtkoFile:(id<NTKOTableDs>)ntkofile Delegate:(id<NTKOPswCheckViewDelegate>)delegate CheckBlock:(PswCheckBlock)block;
@end
