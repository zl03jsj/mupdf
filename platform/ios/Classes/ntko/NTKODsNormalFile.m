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
			_image = [[UIImage imageWithContentsOfFile:_file] retain];
		_title = [[_file pathExtension] retain];
		_describe = [[_file lastPathComponent] retain];
	}
	return self;
}

- (NSData*) data {
	if(!_file)
		return nil;
	return [NSData dataWithContentsOfFile:_file];
}

- (NSData*) imagedata {
	if(!isFileImage(_file))
		return nil;
	return [NSData dataWithContentsOfFile:_file];
}

- (void) dealloc {
	if(_file) [_file release];
	if(_image) [_image release];
	if(_title) [_title release];
	if(_describe) [_describe release];
	[super dealloc];
}

@end
