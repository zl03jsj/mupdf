//
//  SigndeviceViewController_MToken.h
//  MxPDF
//
//  Created by 曾亮 on 9/8/16.
//  Copyright © 2016 Artifex Software, Inc. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "CtnPickerview.h"
#import "../SignkeyFindView.h"
#import "Include/K5SOFApp.h"
#import "Include/K5AlgoDefs.h"

@interface SignkeyMTokenView : SignkeyView
@property (retain, nonatomic) K5SOFApp *k5SOFApp;
@property (copy, nonatomic) NSString *deviceName;
+ (id)loadFromNib;
@end

