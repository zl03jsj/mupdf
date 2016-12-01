//
//  MuSignView.h
//  MxPDF
//
//  Created by 曾亮 on 2016/11/22.
//  Copyright © 2016年 Artifex Software, Inc. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "mupdf/pdf.h"

@interface MuSignView : UIView
@property (copy, nonatomic) NSString* imagefile;
@property (readonly, nonatomic, getter=getSignrect)CGRect signrect;
@property (readonly, assign, nonatomic, getter=getSignAppearance)z_pdf_sign_appearance *signapp;
-(instancetype) initWithPageSize:(CGSize)pagesize;
@end
