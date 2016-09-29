//
//  SigndeviceViewController_MToken.h
//  MxPDF
//
//  Created by 曾亮 on 9/8/16.
//  Copyright © 2016 Artifex Software, Inc. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "CtnPickerview.h"
#import "../SigndeviceDiscoveryView.h"
#import "Include/K5SOFApp.h"
#import "Include/K5AlgoDefs.h"

@interface SigndeviceMTokenView : UIView<SigndeviceView>

@property (retain, nonatomic) K5SOFApp *k5SOFApp;
@property (retain, nonatomic) NSData *signdata;
@property (retain, nonatomic, readonly) NSData *signature;
@property (copy, nonatomic) NSString *deviceName;

+ (id)loadFromNib;

- (void)showMe:(UIView*)parent;

@end
