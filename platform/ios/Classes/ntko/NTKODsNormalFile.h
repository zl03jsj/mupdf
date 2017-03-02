//
//  NKTOSignFile_ImageFile.h
//  MuPDF
//
//  Created by 曾亮 on 2017/2/27.
//  Copyright © 2017年 Artifex Software, Inc. All rights reserved.
//

#import "NTKOTableDs.h"

@interface NTKODsNormalFile : NSObject <NTKOTableDs>

@property (nonatomic, copy) NSString *title;
@property (nonatomic, copy) NSString *describe;
@property (nonatomic, retain, readonly) UIImage *image;

@property (nonatomic, copy) NSString *file;

- (instancetype) initWithFile: (NSString *)file;
@end
