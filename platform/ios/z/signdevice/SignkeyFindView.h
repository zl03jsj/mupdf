//
//  SigndeviceDiscoveryView.h
//  MxPDF
//
//  Created by 曾亮 on 22/09/2016.
//  Copyright © 2016 Artifex Software, Inc. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "../../common.h"
#import "mtoken/Include/K5SOFApp.h"
#import "SignDevice.h"

@interface SignkeyFindView : UIView<K5SOFAppDelegate>
@property (assign, nonatomic, readonly) key_sign_error errcode;
@property (retain, nonatomic) SignkeyView *signkeyView;
+ (id) loadFromNib;
@end
