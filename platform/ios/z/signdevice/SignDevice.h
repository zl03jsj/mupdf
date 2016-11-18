//
//  SignDevice.h
//  MxPDF
//
//  Created by 曾亮 on 2016/11/16.
//  Copyright © 2016年 Artifex Software, Inc. All rights reserved.
//

#ifndef SignDevice_h
#define SignDevice_h

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

enum key_sign_error_s
{
	sign_code_ok = 0,
	sign_code_cancel,
	sign_code_no_device,
	sign_code_invalid_data,
	sign_code_unkown
};

typedef enum key_sign_error_s key_sign_error;

@protocol key_sign_device <NSObject>
@property (retain, nonatomic) NSData *signdata;
@property (retain, nonatomic) NSData *digest;
@property (assign, nonatomic) key_sign_error errcode;

@required
- (NSData*) dosign : (NSData*)digest;

@end

@interface SignkeyView : UIView<key_sign_device>
- (NSData*) dosign : (NSData*)digest;
@end

#endif /* SignDevice_h */
