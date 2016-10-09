//
//  ContainerPickerview.h
//  MxPDF
//
//  Created by 曾亮 on 9/14/16.
//  Copyright © 2016 Artifex Software, Inc. All rights reserved.
//

#import <UIKit/UIKit.h>

typedef void(^VALUE_SELECTED_CB)(NSString *);

@interface CtnPickerview : UIView

/**Picker的标题*/
@property (nonatomic, copy) NSString * title;

/**滚轮上显示的数据(必填,会根据数据多少自动设置弹层的高度)*/
@property (nonatomic, strong) NSArray * datasource;

/**设置默认选项,格式:选项文字/id (先设置dataArr,不设置默认选择第0个)*/
@property (nonatomic, strong) NSString * defaultStr;

/**回调选择的状态字符串(stateStr格式:state/row)*/
// @property (nonatomic, copy) void (^valueDidSelect)(NSString * value);
@property (nonatomic, copy) VALUE_SELECTED_CB valueDidSelect;
/**显示时间弹层*/
- (void)show;

@end
