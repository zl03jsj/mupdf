//
//  NTKOBaseVuew.m
//  MuPDF
//
//  Created by 曾亮 on 2017/2/24.
//  Copyright © 2017年 Artifex Software, Inc. All rights reserved.
//

#import "NTKOBaseView.h"
#import "MuDocumentController.h"

@implementation NTKOBaseView 

- (instancetype)init {
	NSLog(@"Please use initWithTarget to init instance");
	return nil;
}

- (instancetype) initWithTarget:(id)docVc ViewController:(id)vc
{
	self = [super init];
	if(self) {
		_docVc = docVc;
		_vc = vc;
	}
	return self;
}

-(void) initNew:(MuDocumentController*)docVc VC:(UIViewController*)vc {
	// for protecting cycle retain
	// set as assign!!!
	_docVc = docVc;
	_vc = vc;
}

/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect {
    // Drawing code
}
*/

@end
