//
//  NTKOViewController.m
//  MuPDF
//
//  Created by 曾亮 on 2017/2/22.
//  Copyright © 2017年 Artifex Software, Inc. All rights reserved.
//

#import "NTKOViewController.h"
#import "NTKOLoginView.h"
#import "MuDocumentController.h"

@interface NTKOViewController ()
@end

@implementation NTKOViewController {
	MuDocumentController *_target;
	int _viewindex;
}

- (instancetype) initWithTarget:(id)atarget viewIndex:(int)index {
	self = [super init];
	if(self) {
		_target = atarget;
		_viewindex = index > 0 ? index: 0;
	}
	return self;
}

- (void) viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view from its nib.
	NSArray *nib = [[NSBundle mainBundle] loadNibNamed:@"NTKOViews" owner:nil options:nil];
	if(nib.count>_viewindex) {
		NTKOBaseView *baseView = [nib objectAtIndex:_viewindex];
		[baseView initNew:_target VC:self];
		self.view = baseView;
	}
}

- (void)dealloc {
	[super dealloc];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
