//
//  SigndeviceViewController_MToken.m
//  MxPDF
//
//  Created by 曾亮 on 9/8/16.
//  Copyright © 2016 Artifex Software, Inc. All rights reserved.
//
#import "SignkeyMTokenView.h"

@interface SignkeyMTokenView() <K5SOFAppDelegate> {
	NSString *_deviceName;
	NSString *_containerName;
	NSMutableArray *_containers;
	NSInteger _signkeyLen;
	NSInteger _exchkeyLen;
	CtnPickerview *_picker;
}

@property (retain, nonatomic) IBOutlet UILabel *deviceNameLabel;
@property (retain, nonatomic) IBOutlet UILabel *devVersionLabel;
@property (retain, nonatomic) IBOutlet UITextField *authcodeTextfield;
@property (retain, nonatomic) IBOutlet UILabel *deviceSnLabel;
@property (retain, nonatomic) IBOutlet UITextField *userCodeTextfield;
@property (retain, nonatomic) IBOutlet UILabel *containerInfoLabel;
@property (retain, nonatomic) IBOutlet UILabel *messageLabel;
@property (retain, nonatomic) IBOutlet UIButton *openDeviceButton;
@property (retain, nonatomic) IBOutlet UIButton *openContainerButton;
@property (retain, nonatomic) IBOutlet UIActivityIndicatorView *indicator;
@property (retain, nonatomic) IBOutlet UIButton *signButton;
@property (retain, nonatomic) IBOutlet UIButton *cancelButton;

@end

@implementation SignkeyMTokenView

@synthesize deviceName = _deviceName;

#pragma mark - Override UIVIew
- (instancetype)initWithFrame:(CGRect)frame
{
	NSLog(@"%s",__func__);
	self = [super initWithFrame:frame];
	if (self) {
		_k5SOFApp = nil;
		self.backgroundColor = [UIColor whiteColor];
		self.alpha = 0.5;
	}
	return self;
}

+ (id) loadFromNib {
	NSLog(@"%s",__func__);
	SignkeyMTokenView *view = [[[NSBundle mainBundle] loadNibNamed:@"SignkeyMTokenView" owner:nil options:nil] lastObject];
	return view;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
	NSLog(@"%s",__func__);
	self = [super initWithCoder:coder];
	_k5SOFApp = nil;
	_picker = nil;
	return self;
}

// 使用xib的时候,建议在awakeFromNib中设置视图的属性
-(void)awakeFromNib {
	NSLog(@"%s",__func__);
	[super awakeFromNib];
	self.backgroundColor = [UIColor whiteColor];
	self.alpha = 0.5;

	_openDeviceButton.enabled = FALSE;
	_openContainerButton.enabled = FALSE;
	_signButton.enabled = FALSE;
	_picker = [[CtnPickerview alloc]init];
}

//////////////////////////////////
#pragma mark - IBACTIONS
- (IBAction)connectDeviceTaped:(id)sender {
	if(!_k5SOFApp) {
		NSLog(@"device K5SOFApp not initialnized.");
		return;
	}
	_messageLabel.text = @"connectiong to device..";
	[_indicator startAnimating];
	[_k5SOFApp SOF_ConnectAsyn:_deviceNameLabel.text authCode:_authcodeTextfield.text application:@"NTKOPDFSignApplication"];
}

- (IBAction)openContainerTaped:(id)sender {
	[self chooseContainer];
}

- (IBAction)dosignTaped:(id)sender {
	NSInteger i = 0;
	[self setStatus:@"Logining in..." showAnimation:YES];
	
	if( 0!=[_k5SOFApp SOF_Login:_userCodeTextfield.text leftRetry:&i] ) {
		[self setStatus:@"Login failed.Please check user code!" showAnimation:NO];
		return;
	}
	[self setStatus:@"Login Success.Calculating digest..." showAnimation:YES];
	
	NSInteger signflag = -1;
	if( !(((int)_signkeyLen)==0) ) {
		signflag = 1;
	}
	else if( !(((int)_exchkeyLen)==0) ) {
		signflag = 0;
	}
	
	if( -1==signflag ) {
		[self setStatus:@"No key pair." showAnimation:NO];
		return;
	}
	
	NSInteger hashAlgo = SGD_SHA1;
	
	// the digest property is already caculated use sha1 algorithm
	// no need to caculate agian.
#if 0
	if( 0!=[_k5SOFApp SOF_DigestInit_Display:hashAlgo display:YES] ||
	    0!=[_k5SOFApp SOF_DigestUpdate_Display:self.digest display:YES] ) {
		[self setStatus:@"Calculate SHA1 Failed" showAnimation:NO];
		return;
	}
#endif
	
	[self setStatus:@"Doing sign..." showAnimation:YES];
	NSMutableData *signature = [[NSMutableData alloc]init];
	if( 0!=[_k5SOFApp SOF_SignData:_containerName signFlag:signflag hashAlgo:hashAlgo timeOut:20 data:nil signature:signature] ) {
		[self setStatus:@"Do sign failed." showAnimation:NO];
	}
	else {
		[self setStatus:@"Done!" showAnimation:NO];
	}
	self.signdata = signature;
	[signature release];
}

- (IBAction)cancelTaped:(id)sender {
	NSLog(@"self retainCount=%d",(int)[self retainCount]);
	[self removeFromSuperview];
}

#pragma mark - help functions

- (void)refreshDeviceInfomation {
	NSMutableString * dname = [[NSMutableString alloc]init];
	NSMutableString * sn = [[NSMutableString alloc]init];
	NSMutableString * version = [[NSMutableString alloc]init];
	
	[self setStatus:@"geting device information..." showAnimation:YES];
	if ( 0!=[_k5SOFApp SOF_GetDeviceInfo:dname SerialNumber:sn Version:version] ) {
		_deviceNameLabel.text = dname;
		_devVersionLabel.text = version;
		_deviceSnLabel.text = sn;
		[self setStatus:@"get devive information ok." showAnimation:NO];
	}
	else {
		[self setStatus:@"get device information failed." showAnimation:NO];
	}
	
	NSLog(@"dname's retaincount=%d", (int)[dname retainCount]);

	[dname release];
	[sn release];
	[version release];
}

- (void)refreshContainerInfomation {
	
	NSInteger type, signCertFlag, exchCertFlag;
	[self setStatus:[[NSString stringWithFormat:@"geting container(%@) information...",_containerName] autorelease]
	  showAnimation:YES];
	if( 0 != [_k5SOFApp SOF_GetContainerInfo:_containerName
							  containerType:&type
								 signKeyLen:&_signkeyLen
								 exchKeyLen:&_exchkeyLen
							   signCertFlag:&signCertFlag
							   exchCertFlag:&exchCertFlag] )
	{
		[self setStatus: [[NSString stringWithFormat:@"geting container(%@) information failed.",_containerName] autorelease] showAnimation:NO];
		return;
	}
	NSString *info = [NSString stringWithFormat:@"ContainerName:%@,type:%d\n"
					  "key pair:signature:%d,key exchange:%d\n"
					  "cert: signature:%d,ekey exchange:%d",
					  _containerName,(int)type,(int)_signkeyLen,(int)_exchkeyLen,
					  (int)signCertFlag,(int)exchCertFlag];
	_containerInfoLabel.text = info;
	
	[self setStatus:[[NSString stringWithFormat:@"get container(%@) information ok!",_containerName] autorelease] showAnimation:NO];
	[info release];
}

- (void) chooseContainer {
	[_indicator startAnimating];
	[_containers removeAllObjects];
	if(0 != [_k5SOFApp SOF_EnumContainers:_containers])	{
		[self setStatus:@"container not found." showAnimation:NO];
		return ;
	}
	_picker.datasource = [_containers retain];
	_picker.title = @"select sign cert container";
	
	SignkeyMTokenView *refself = self;
	_picker.valueDidSelect = ^(NSString *value) {
		refself->_containerName = [value copy];
		[refself refreshContainerInfomation];
	};
	[_picker show];
}

- (void)setStatus:(NSString*)statusstr showAnimation:(BOOL)showAnimation {
	_messageLabel.text = statusstr;
	if ( [_indicator isAnimating]!=showAnimation ) {
		if(showAnimation) {
			[_indicator startAnimating];
		}else {
			[_indicator stopAnimating];
		}
	}
}
//////////////////////////////////
#pragma mark - K5SOFAppDelegate
-(void) didDeviceDiscovered:(NSString * )devName
{
	_authcodeTextfield.text = devName;
	printf("K5SOFApp [%s]:didDeviceDiscovered!", [devName UTF8String]);
}

-(void) didDeviceConnected:(NSString *)devName  error:(NSError *)error
{
	_messageLabel.text = [NSString stringWithFormat:@"%@ connected.",devName];
	[_indicator stopAnimating];
	[self refreshDeviceInfomation];
	_openContainerButton.enabled = TRUE;
	printf("K5SOFApp [%s]:didDeviceConnected!", [devName UTF8String]);
	
}

-(void) didDeviceDisconnected:(NSString*)devName
{
	_openDeviceButton.enabled = FALSE;
	_openContainerButton.enabled = FALSE;
	_signButton.enabled = FALSE;
	
	_messageLabel.text = [NSString stringWithFormat:@"%@ disconnected.",devName];
	printf("K5SOFApp [%s]:didDeviceDisconnected!", [devName UTF8String]);
}

/*
#pragma mark - UITableViewDataSource implementation
- (UITableViewCell *)tableView:(UITableView *)tableView
		 cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	
	static NSString *TableSampleIdentifier = @"TableSampleIdentifier";
	
	UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:
							 TableSampleIdentifier];
	if ( !cell ) {
		cell = [[UITableViewCell alloc]
				initWithStyle:UITableViewCellStyleDefault
				reuseIdentifier:TableSampleIdentifier];
	}
	
	NSUInteger row = [indexPath row];
	cell.textLabel.text = [self.list objectAtIndex:row];
	return cell;
}

#pragma mark - UITableViewDelegate implementation
-(void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	// NSString *rowString = [self.list objectAtIndex:[indexPath row]];
    NSString *rowString = @"hello!";
    UIAlertView * alter = [[UIAlertView alloc]
						   initWithTitle:@"选中的行信息"
						   message:rowString
						   delegate:self
						   cancelButtonTitle:@"确定"
						   otherButtonTitles:nil,
						   nil];
	[alter show];
}
*/

- (void)dealloc {
	[_deviceNameLabel release];
	[_authcodeTextfield release];
	[_deviceSnLabel release];
	[_devVersionLabel release];
	[_userCodeTextfield release];
	[_containerInfoLabel release];
	[_messageLabel release];
	[_openDeviceButton release];
	[_openContainerButton release];
	[_indicator release];
	[_signButton release];
	[_cancelButton release];
	[_k5SOFApp SOF_Disconnect];
	[_k5SOFApp release];

	[_deviceName release];
	[_containerName release];
	[_containers release];
	[_picker release];
	
    [super dealloc];
}
@end
