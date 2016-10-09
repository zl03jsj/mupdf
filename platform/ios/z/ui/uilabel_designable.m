//
//  uilabel_designable.m
//  MxPDF
//
//  Created by 曾亮 on 9/19/16.
//  Copyright © 2016 Artifex Software, Inc. All rights reserved.
//

#import "uilabel_designable.h"

@implementation uilabel_designable

/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect {
    // Drawing code
}
*/
-(void) setBorderWidth:(CGFloat)borderWidth
{
	if(borderWidth<0) return;
	self.layer.borderWidth = borderWidth;
}

-(void) setBorderColor:(UIColor *)borderColor
{
	_borderColor = borderColor;
	self.layer.borderColor = borderColor.CGColor;
}

-(void) setCornerRadius:(CGFloat)cornerRadius
{
	self.layer.cornerRadius = cornerRadius;
	self.layer.masksToBounds= cornerRadius > 0;
}
@end
