//
//  NTKOPfxFile.h
//  MuPDF
//
//  Created by 曾亮 on 2017/3/7.
//  Copyright © 2017年 Artifex Software, Inc. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "NTKODsNormalFile.h"

@interface NTKOPfxFile : NTKODsNormalFile
// the returened void* pointer is a z_device pointer
// must call z_drop_device to free
- (void*)createOpsslDev:(NSString*)password;
@end
