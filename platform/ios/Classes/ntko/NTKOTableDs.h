//
//  NTKOSignFile.h
//  MuPDF
//
//  Created by 曾亮 on 2017/2/27.
//  Copyright © 2017年 Artifex Software, Inc. All rights reserved.
//

#ifndef NTKOTableDs_h
#define NTKOTableDs_h

@protocol NTKOTableDs<NSObject>
@required
@property (nonatomic, copy) NSString *title;
@property (nonatomic, copy) NSString *describe;
@property (nonatomic, retain, readonly) UIImage *image;

@end

#endif /* NTKOSignFile_h */
