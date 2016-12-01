//
//  MuHanddrawView.h
//  MxPDF
//
//  Created by 曾亮 on 2016/11/23.
//  Copyright © 2016年 Artifex Software, Inc. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface MuHanddrawView : UIView

@property (retain, nonatomic)   UIColor *color;
@property (readonly, nonatomic) NSArray *curves;

- (id) initWithPageSize:(CGSize)pageSize;

@end
