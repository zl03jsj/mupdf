//
//  SigndeviceDiscoveryView.h
//  MxPDF
//
//  Created by 曾亮 on 22/09/2016.
//  Copyright © 2016 Artifex Software, Inc. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "mtoken/Include/K5SOFApp.h"

@protocol SigndeviceView <NSObject>
@required
- (void) showMe:(UIView *)parent;
@end


@interface SigndeviceDiscoveryView : UIView <K5SOFAppDelegate>
+ (id) loadFromNib;
@end
