//
//  uibutton_designable.h
//  MxPDF
//
//  Created by 曾亮 on 9/13/16.
//  Copyright © 2016 Artifex Software, Inc. All rights reserved.
//

#import <UIKit/UIKit.h>

IB_DESIGNABLE
@interface uibutton_designable : UIButton
@property (assign, nonatomic) IBInspectable CGFloat borderWidth;
@property (strong, nonatomic) IBInspectable UIColor *borderColor;
@property (assign, nonatomic) IBInspectable CGFloat cornerRadius; 
@end
