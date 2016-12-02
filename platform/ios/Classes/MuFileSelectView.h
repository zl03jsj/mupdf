//
//  MuSignGetDeviceView.h
//  MxPDF
//
//  Created by 曾亮 on 2016/11/28.
//  Copyright © 2016年 Artifex Software, Inc. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "mupdf/pdf.h"

@protocol MuFileSelectViewDelegate;

@interface MuFileSelectView : UIView

@property (nonatomic,assign) id <MuFileSelectViewDelegate> delegate;

@property (retain,nonatomic) NSArray * suffixs;
@property (retain,nonatomic) NSArray * paths;
@property (assign,nonatomic) BOOL showimage;
@property (nonatomic,readonly) NSString * currentfile;
@property (copy, nonatomic) NSString *title;
-(instancetype) initWithPageSize:(CGSize)pagesize filesunderpaths: (NSArray*)paths filehassuffix: (NSArray*)suffix;
+(instancetype) showFileSelectView:(UIView*)view pagesize:(CGSize)size viewframe:(CGRect)frame filesunderpaths:(NSArray*)paths filehassuffix:(NSArray*)suffix viewtitle:(NSString*)title showimage:(BOOL)isshow fileselDelegate:(id<MuFileSelectViewDelegate>)delegate;

+(instancetype) showDefaultImageSelectView: (UIView*)view pagesize:(CGSize)size viewframe:(CGRect)frame fileselDelegate:(id<MuFileSelectViewDelegate>)delegate;
+(instancetype) showDefaultPfxSelectView: (UIView*)view pagesize:(CGSize)size viewframe:(CGRect)frame fileselDelegate:(id<MuFileSelectViewDelegate>)delegate;

@end

@protocol MuFileSelectViewDelegate
@optional
- (void)fileSelected:(MuFileSelectView*)fileselView selectedfile:(NSString *)file;
@end
