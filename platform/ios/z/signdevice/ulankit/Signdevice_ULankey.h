//
//  Signdevice_ULankey.h
//  MxPDF
//
//  Created by 曾亮 on 9/5/16.
//  Copyright © 2016 Artifex Software, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Signdevice_ULankey : NSObject
+ (id) getInstance;
- (NSData*) sign:(NSData*)dataTosign;
@end


