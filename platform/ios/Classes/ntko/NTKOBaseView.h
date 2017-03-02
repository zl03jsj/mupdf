//
//  NTKOBaseView.h
//  MuPDF
//
//  Created by 曾亮 on 2017/2/24.
//  Copyright © 2017年 Artifex Software, Inc. All rights reserved.
//

#import <UIKit/UIKit.h>

@class MuDocumentController;

@interface NTKOBaseView : UIView {
@protected
	MuDocumentController *_docVc;
	UIViewController *_vc;
}

-(void) initNew:(MuDocumentController*)docVc VC:(UIViewController*)vc;

@end
