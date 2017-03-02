//
//  MuSignView.h
//  MxPDF
//
//  Created by 曾亮 on 2016/11/22.
//  Copyright © 2016年 Artifex Software, Inc. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "mupdf/pdf.h"

@protocol MuSignViewDelegate
@optional
- (void)imagePositionOk: (CGRect)imagerectOnPage;
- (void)imagePositionNotOk;
@end

@protocol NTKOTableDs;

@interface MuSignView : UIView
@property (retain, nonatomic) id<MuSignViewDelegate> delegate;
@property (nonatomic, retain, setter=setSignFile:) id<NTKOTableDs> signfile;
@property (readonly, nonatomic, getter=getRectOfPage)CGRect rectOfPage;
-(instancetype) initWithPageSize:(CGSize)pagesize;
@end
