//
//  MuFileselectViewController.h
//  MxPDF
//
//  Created by 曾亮 on 2016/12/6.
//  Copyright © 2016年 Artifex Software, Inc. All rights reserved.
//

#import <UIKit/UIKit.h>

@class MuFileselectViewController;
@protocol NTKOTableDs;

typedef BOOL(^FileSelectedBlock)(MuFileselectViewController *vc, id<NTKOTableDs> ds, NSIndexPath *inexpath);

@protocol MuFileSelectViewDelegate
@optional
- (BOOL)fileSelected:(MuFileselectViewController*)vc Selected:(id<NTKOTableDs>)selDs selectdIndexpath:(NSIndexPath*)indexpath;
- (void)fileSelCancel:(MuFileselectViewController*)vc;
- (void)fileSelOkay:(MuFileselectViewController*)vc Selected:(id<NTKOTableDs>)selds;
@end

@interface MuFileselectViewController : UIViewController

@property (nonatomic, assign) id<MuFileSelectViewDelegate> fileselDelegate;
@property (nonatomic, copy) FileSelectedBlock fileselectedBlock;
@property (nonatomic, assign) id backViewController;

- (instancetype) initWithDatasource:(NSArray<id<NTKOTableDs>>*)ds Title:(NSString*)title FileSelDelegate:(id<MuFileSelectViewDelegate>)delegate FileSelBlock:(FileSelectedBlock)block;

- (void) addTableSection:(NSArray<id<NTKOTableDs>>*)sectionDs Title:(NSString*)title;

+ (void) pushDefaultImageSelect: (UIViewController*)parentVC SelDelegate:(id<MuFileSelectViewDelegate>)delegate SelBlock:(FileSelectedBlock)block;

+ (void)showDefaultImageSelect: (UIViewController*)parentVC SelDelegate:(id<MuFileSelectViewDelegate>)delegate SelBlock:(FileSelectedBlock)block;

+ (void) pushDefaultPfxSelect: (UIViewController*)parentVC fileselectedDelegate:(id<MuFileSelectViewDelegate>)delegate fileselectedblock:(FileSelectedBlock)block;

+ (void)showDefaultPfxSelect: (UIViewController*)parentVC SelDelegate:(id<MuFileSelectViewDelegate>)delegate SelBlock:(FileSelectedBlock)block;

+ (instancetype) defaultImageViewController: (id<MuFileSelectViewDelegate>)delegate SelBlock:(FileSelectedBlock)block;

+ (instancetype) defaultPfxViewController: (id<MuFileSelectViewDelegate>)delegate SelBlock:(FileSelectedBlock)block;

+ (NSArray*) defaultImages;
+ (NSArray*) defaultPfxs;
+ (NSArray*) defaultSigns;

@end
