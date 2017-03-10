//
//  NTKOSignSvrSettings.h
//  MuPDF
//
//  Created by 曾亮 on 2017/2/22.
//  Copyright © 2017年 Artifex Software, Inc. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "NTKOBaseView.h"

@protocol NTKOLoginDelegate
@optional
- (void)OnLoginOk;
@end

typedef void (^BLOCK_LoginOk)();

@interface NTKOLoginView : NTKOBaseView
@property (nonatomic, copy) BLOCK_LoginOk block;
@property (nonatomic, assign) id<NTKOLoginDelegate> delegate;
@end
