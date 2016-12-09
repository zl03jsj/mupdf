//
//  MuFileselectViewController.h
//  MxPDF
//
//  Created by 曾亮 on 2016/12/6.
//  Copyright © 2016年 Artifex Software, Inc. All rights reserved.
//

#import <UIKit/UIKit.h>
@class MuFileselectViewController;

typedef BOOL(^FileSelectedBlock)(MuFileselectViewController *vc, NSString *selectedfile, NSIndexPath *inexpath);

@protocol MuFileSelectViewDelegate
@optional
- (BOOL)fileSelected:(MuFileselectViewController*)fileselectViewController selectedfile:(NSString*)file selectdIndexpath:(NSIndexPath*)indexpath;
- (void)fileSelCancel:(MuFileselectViewController*)fileselectViewController;
- (void)fileSelOkay:(MuFileselectViewController*)fileselectViewController selectedfile:(NSString*)file;
@end

@interface MuFileselectViewController : UIViewController
@property (nonatomic, assign) id<MuFileSelectViewDelegate> fileselDelegate;
@property (nonatomic, copy) FileSelectedBlock fileselectedBlock;
- (instancetype)initWithChoices:(NSString*)title userSection:(NSArray *)section userSectionTitle:(NSString*)sectionTitle filesuffixs:(NSArray*)suffixs filepaths:(NSArray*)paths isshowimage:(BOOL)isshow fileselectedDelegate:(id<MuFileSelectViewDelegate>)delegate fileselectedblock:(FileSelectedBlock)fileselblock;

+ (void)showDefaultPfxSelect: (UIViewController*)parentVC fileselectedDelegate:(id<MuFileSelectViewDelegate>)delegate fileselectedblock:(FileSelectedBlock)fileselblock;
+ (void)showDefaultImageSelect: (UIViewController*)parentVC fileselectedDelegate:(id<MuFileSelectViewDelegate>)delegate fileselectedblock:(FileSelectedBlock)fileselblock;

@end
