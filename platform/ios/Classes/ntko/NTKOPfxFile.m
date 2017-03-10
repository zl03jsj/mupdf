//
//  NTKOPfxFile.m
//  MuPDF
//
//  Created by 曾亮 on 2017/3/7.
//  Copyright © 2017年 Artifex Software, Inc. All rights reserved.
//

#import "NTKOPfxFile.h"
#include "common.h"

@implementation NTKOPfxFile
- (void*)createOpsslDev:(NSString*)password {
	z_device *device = NULL;
	NSString *pfxfile = self.file;
	fz_try(ctx) {
		device = z_openssl_new_device(ctx, (char*)[pfxfile cStringUsingEncoding:NSUTF8StringEncoding], (char*)[password cStringUsingEncoding:NSUTF8StringEncoding]);
	}
	fz_catch(ctx) {
		NSLog(@"%s", fz_caught_message(ctx));
	}
	return device;
}

- (oneway void) release {
	NSLog(@"retain count=%ld", [self retainCount]);
	[super release];
}

- (void) dealloc {
	NSLog(@"%@ was dealloced", [self class]);
	[super dealloc];
}
@end
