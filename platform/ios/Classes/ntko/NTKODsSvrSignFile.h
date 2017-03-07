//
//  NTKOSignFile_espfile.h
//  MuPDF
//
//  Created by 曾亮 on 2017/2/27.
//  Copyright © 2017年 Artifex Software, Inc. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "NTKOTableDs.h"
#include "common.h"

@interface NTKODsSvrSignFile : NSObject <NTKOTableDs>

@property (nonatomic, copy) NSString *title;
@property (nonatomic, copy) NSString *describe;
@property (nonatomic, retain, readonly) NSData *data;
@property (nonatomic, retain, readonly) NSData *imagedata;
@property (nonatomic, retain, readonly) UIImage *image;

@property (nonatomic, copy) NSString *name;
@property (nonatomic, copy) NSString *sn;
@property (nonatomic, copy) NSString *unid;
@property (nonatomic, copy) NSString *user;

@property (nonatomic, copy) NSString *signer;

- (instancetype) initWithSvrEspInfo:(ntko_server_espinfo*) espinfo;
- (BOOL)open:(NSString *)password;

+ (NSArray<NTKODsSvrSignFile*>*) svrEsplist;
@end
