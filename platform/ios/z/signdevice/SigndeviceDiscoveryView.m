//
//  SigndeviceDiscoveryView.m
//  MxPDF
//
//  Created by 曾亮 on 22/09/2016.
//  Copyright © 2016 Artifex Software, Inc. All rights reserved.
//

#import "SigndeviceDiscoveryView.h"
#import "mtoken/SigndeviceMTokenView.h"

static NSArray *supprotedDevicenames;

@interface SigndeviceDiscoveryView()<K5SOFAppDelegate> {
}

@property (retain, nonatomic) IBOutlet UIActivityIndicatorView *indicator;
@property (retain, nonatomic) IBOutlet UILabel *messageLb;

@end

@implementation SigndeviceDiscoveryView  {
	NSMutableDictionary *_devices;
	NSString *_connectedDeivceclass;
	UIView *_deviceView;
}

+ (id)loadFromNib {
	NSArray *objs = [[NSBundle mainBundle]loadNibNamed:@"SignDeviceDiscoveryView" owner:nil options:nil];
	for( id obj in objs ) {
		NSLog(@"objtype=%@", NSStringFromClass([obj class]));
	}
	SigndeviceDiscoveryView *view = [objs firstObject];
	return view;
}
// call before Use SigndeviceDiscoveryView
// init golobal vars, only used in this source file.
+ (void)initialize {
	supprotedDevicenames = @[NSStringFromClass([self class])];
}

// call before main
// init golobal variables, may be used in other file.
+ (void)load {

}

- (void)privateInit {
	_devices = nil;
	_connectedDeivceclass = nil;
	_deviceView = nil;
	
	_devices = [[NSMutableDictionary alloc]init];
	
	K5SOFApp *k5sofapp = [[K5SOFApp alloc]init];
	k5sofapp.delegate = self;
	
	_devices[NSStringFromClass([k5sofapp class])] = k5sofapp;
	[k5sofapp release];
	NSLog(@"retainCount of k5sofapp=%d",(int)[k5sofapp retainCount]);
	
	// other device
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
	NSLog(@"%s",__func__);
	self = [super initWithCoder:coder];
	return self;
}

-(void)awakeFromNib {
	[super awakeFromNib];
	[self privateInit];
}

- (IBAction)beginTaped:(id)sender {
	if( !_devices || ![_devices count] ){
		[self setMessage:@"cannot start scan device, no deivce class is created." animation:NO];
		return;
	}
	
	NSMutableString *msg = [[NSMutableString alloc]init];
	NSMutableArray *scaningDevices = [[NSMutableArray alloc]init];
	
#pragma message("create and start device scanning here...")
	K5SOFApp *k5sofapp = _devices[NSStringFromClass([K5SOFApp class])];
	printf(">>>>>get k5sofapp from array, k5sofapp refcount is:%d\n", (int)[k5sofapp retainCount]);

	if( k5sofapp && 0==[k5sofapp SOF_StartEnumDevices] ) {
		[scaningDevices addObject:NSStringFromClass([K5SOFApp class])];
	}
	
	// #pragma message("if need release object get from dictionary???? needs to comfirm!")
	// no need to release
	// [k5sofapp release];
	
	// set message
	if( scaningDevices.count ) {
		[msg appendFormat:@"%d devices start scaning:",(int)scaningDevices.count];
		for (id obj in scaningDevices) {
			[msg appendString:(NSString*)obj];
			[msg appendString:@","];
		}
		[self setMessage:msg animation:YES];
	}
	
	[scaningDevices release];
	[msg release];
}

- (IBAction)okTaped:(id)sender {
	if(nil==_connectedDeivceclass) {
		return;
	}
	[self showDeviceView];
}

- (IBAction)cancelTaped:(id)sender {
	[self removeFromSuperview];
}

- (void)setMessage:(NSString*)msg animation:(BOOL)isshowAnimation
{
	_messageLb.text = msg;
	if( _indicator.isAnimating != isshowAnimation ) {
		if(isshowAnimation==YES) [_indicator startAnimating];
		else [_indicator stopAnimating];
	}
}

- (void)showDeviceView {
	if( nil==_deviceView ) {
		printf("deviceView is nil");
		return;
	}
	UIView *superView = self.superview;
	[self removeFromSuperview];
	float pading = 20.0f;
	_deviceView.frame = CGRectInset(superView.bounds, pading, pading);
	[superView addSubview:_deviceView];
}

#pragma mark - K5SOFAppDelegate
-(void) didDeviceDiscovered:(NSString * )devName
{
	if( _connectedDeivceclass ){
#pragma message("need to confirm if need release _connnectedDeviceclass")
		[_connectedDeivceclass release];
	}
	if( _deviceView ){
		[_deviceView release];
	}
	
	_connectedDeivceclass = NSStringFromClass([K5SOFApp class]);

	SigndeviceMTokenView *devView = [SigndeviceMTokenView loadFromNib];
	NSLog(@"MTokenView retainCount=%d",(int)[devView retainCount]);
	
	devView.k5SOFApp = [_devices objectForKey:_connectedDeivceclass];
	// set to sign data here!!!!
	devView.signdata = nil;
	devView.deviceName = devName;
	
	_deviceView = [devView retain];
	
	[self setMessage:[NSString stringWithFormat:@"Mtoken bluetoothEkey:%@ discovery.",devName] animation:NO];
	printf("K5SOFApp [%s]:didDeviceDiscovered!", [devName UTF8String]);
}

-(void) didDeviceConnected:(NSString *)devName  error:(NSError *)error
{
	printf("K5SOFApp [%s]:didDeviceConnected!", [devName UTF8String]);
}

-(void) didDeviceDisconnected:(NSString*)devName
{
	printf("K5SOFApp [%s]:didDeviceDisconnected!", [devName UTF8String]);
}
- (void)dealloc {
	[_indicator release];
#pragma message("needs to comfirming if _connectedDeivceclass needs to release")
	[_connectedDeivceclass release];
	NSLog(@"%@,retainCount=%d", _connectedDeivceclass,(int)[_connectedDeivceclass retainCount]);
	[_messageLb release];
	if(_deviceView!=nil) [_deviceView release];
	if(_devices!=nil) {
		[_devices removeAllObjects];
		[_devices release];
	}
	[_connectedDeivceclass release];
	[super dealloc];
}
@end
