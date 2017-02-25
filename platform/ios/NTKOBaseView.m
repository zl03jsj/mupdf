//
//  NTKOBaseVuew.m
//  MuPDF
//
//  Created by 曾亮 on 2017/2/24.
//  Copyright © 2017年 Artifex Software, Inc. All rights reserved.
//

#import "NTKOBaseView.h"
#import "MuDocumentController.h"

@implementation NTKOBaseView {
	MuDocumentController *_target;
}

- (instancetype)init {
	NSLog(@"Please use initWithTarget to init instance");
	return nil;
}

- (instancetype) initWithTarget:(id)target {
	self = [super init];
	if(self) {
		_target = target;
	}
	return self;
}

/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect {
    // Drawing code
}
*/

@end
