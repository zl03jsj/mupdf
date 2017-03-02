//
//  NKTOSignFile_ImageFile.m
//  MuPDF
//
//  Created by 曾亮 on 2017/2/27.
//  Copyright © 2017年 Artifex Software, Inc. All rights reserved.
//

#import "common.h"
#import "NTKODsNormalFile.h"

@implementation NTKODsNormalFile {
	NSString *_file;
}

- (instancetype) initWithFile:(NSString *)file {
	self = [super init];
	if(self) {
		self->_file = [file retain];
		if(isFileImage(_file))
			self->_image = [[UIImage imageWithContentsOfFile:_file] retain];
	}
	return self;
}

- (void) dealloc {
	if(_file) [_file release];
	if(_image) [_image release];
	[super dealloc];
}

@end
