//
//  ViewController.m
//  uidesignable
//
//  Created by 曾亮 on 9/12/16.
//  Copyright © 2016 Artifex Software, Inc. All rights reserved.
//

#import "ViewController.h"
#import "../z/signdevice/SignkeyFindView.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
	[super viewDidLoad];
	NSLog(@"%s",__func__);
	SigndeviceDiscoveryView *subView = [[[NSBundle mainBundle]loadNibNamed:@"SignDeviceDiscoveryView" owner:nil options:nil]lastObject];
	subView.frame = CGRectInset(self.view.bounds, 20, 40);
	[self.view addSubview:subView];
}

- (void)didReceiveMemoryWarning {
	[super didReceiveMemoryWarning];
	// Dispose of any resources that can be recreated.
}

@end
