//
//  MuSignGetDeviceView.m
//  MxPDF
//
//  Created by 曾亮 on 2016/11/28.
//  Copyright © 2016年 Artifex Software, Inc. All rights reserved.
//

#import "MuFileSelectView.h"

BOOL isFileImage(NSString *file) {
	NSString const* image_suffix[] = {@".bmp",@".jpeg",@".jpg",@".bmp",@".png", @".gif"};
	for(int i=0; i<sizeof(image_suffix)/sizeof(NSString*); i++)
		if([[file lowercaseString] hasSuffix:(NSString*)image_suffix[i]])
			return YES;
	return NO;
}

BOOL isFilePfx(NSString *file) {
	return [[file lowercaseString]hasSuffix:@".pfx"];
}

BOOL fileIsInPath(NSArray *paths, NSString *file) {
	for(NSString *path in paths) {
		if([[file lowercaseString ]hasPrefix:[path lowercaseString]])
			return YES;
	}
	return NO;
}

BOOL fileHasSuffixs(NSArray *suffixs, NSString *file) {
	for(NSString *suffix in suffixs) {
		if([[file lowercaseString] hasSuffix:[suffix lowercaseString]])
			return YES;
	}
	return NO;
}

@interface MuFileSelectView() <UITableViewDataSource, UITableViewDelegate>
@end

@implementation MuFileSelectView {
	UITableView *_tableView;
	CGSize _pagesize;
	NSMutableArray *_files;
}

-(void) initData {
	if(!_files) _files = [[NSMutableArray alloc]init];
	else [_files removeAllObjects];

	NSFileManager *fileman = [NSFileManager defaultManager];
	NSString *docdir = [NSString stringWithFormat: @"%@/Documents", NSHomeDirectory()];

	NSDirectoryEnumerator *direnum = [fileman enumeratorAtPath:docdir];
	NSString *file;
	BOOL isdir;
	while ( (file = [direnum nextObject]) ) {
		NSString *filepath = [docdir stringByAppendingPathComponent:file];
		if ([fileman fileExistsAtPath:filepath isDirectory:&isdir] && !isdir) {
			if(fileIsInPath(_paths, file) && fileHasSuffixs(_suffixs, file)) {
				[_files addObject:filepath];
			}
		}
	}
	[_tableView reloadData];
}

-(instancetype) initWithPageSize:(CGSize)pagesize filesunderpaths: (NSArray*)paths filehassuffix: (NSArray*)suffix
{
	self = [super initWithFrame:CGRectMake(0, 0, 100, 100)];
	if( !self ) return nil;

	_paths = [paths retain];
	_suffixs = [suffix retain];
	
	_pagesize = pagesize;

	[self setOpaque:YES];
	
	_tableView =[[UITableView alloc]initWithFrame:self.bounds style:UITableViewStyleGrouped];
	
	_tableView.layer.borderColor = [[UIColor colorWithRed:0.8f green:0.8f blue:0.8f alpha:0.8f]CGColor];
	_tableView.layer.borderWidth = 1.0f;
	
	_tableView.separatorStyle = UITableViewCellSeparatorStyleSingleLine;
	_tableView.allowsSelection = true;
	_tableView.dataSource = self;
	_tableView.delegate = self;
	
	[self addSubview:_tableView];
	[self initData];
	
	return self;
}

+(instancetype) showFileSelectView:(UIView*)view pagesize:(CGSize)size viewframe:(CGRect)frame filesunderpaths:(NSArray*)paths filehassuffix:(NSArray*)suffix viewtitle:(NSString*)title showimage:(BOOL)isshow fileselDelegate:(id<MuFileSelectViewDelegate>)delegate
{
	MuFileSelectView *fileselview = [[MuFileSelectView alloc]initWithPageSize:size filesunderpaths:paths filehassuffix:suffix];
	fileselview.title = title;
	fileselview.showimage = isshow;
	fileselview.delegate = delegate;
	
	[fileselview setFrame:frame];
	[view addSubview:fileselview];
	
	return fileselview;
}

+(instancetype) showDefaultImageSelectView: (UIView*)view pagesize:(CGSize)size viewframe:(CGRect)frame fileselDelegate:(id<MuFileSelectViewDelegate>)delegate
{
	NSArray *imagefilesuffixs = [NSArray arrayWithObjects:@".bmp",@".jpeg",@".jpg",@".bmp",@".png", @".gif", nil];
	NSArray *imagefilepaths = [NSArray arrayWithObjects:@"_imagefiles", nil];
	NSString *title = @"选择签名图片文件";
	MuFileSelectView *fileSelview = [MuFileSelectView showFileSelectView:view pagesize:size viewframe:frame filesunderpaths:imagefilepaths filehassuffix:imagefilesuffixs viewtitle:title showimage:YES fileselDelegate:delegate];
	return fileSelview;
}

+(instancetype) showDefaultPfxSelectView: (UIView*)view pagesize:(CGSize)size viewframe:(CGRect)frame fileselDelegate:(id<MuFileSelectViewDelegate>)delegate
{
	NSArray *pfxfilepaths = [NSArray arrayWithObjects:@"_pfxfiles", nil];
	NSArray *pfxfilesuffixs = [NSArray arrayWithObjects:@".pfx", nil];
	NSString *title = @"选择签名数字证书";
	MuFileSelectView *fileSelview = [MuFileSelectView showFileSelectView:view pagesize:size viewframe:frame filesunderpaths:pfxfilepaths filehassuffix:pfxfilesuffixs viewtitle:title showimage:NO fileselDelegate:delegate];
	return fileSelview;
}

-(void) layoutSubviews {
	[super layoutSubviews];
	CGRect frame = CGRectMake(0, 0, self.frame.size.width, self.frame.size.height);
	frame = CGRectInset(frame, 20, 20);
	[_tableView setFrame:frame];
}

- (UITableViewCell*) tableView: (UITableView*)tableView cellForRowAtIndexPath: (NSIndexPath*)indexPath
{
	// int section = (int)indexPath.section;
	int row = (int)indexPath.row;
	UITableViewCell *cell = [_tableView dequeueReusableCellWithIdentifier: @"filecell"];
	if (!cell)
		cell = [[[UITableViewCell alloc] initWithStyle: UITableViewCellStyleSubtitle reuseIdentifier: @"filecell"] autorelease];
	
	NSString *filename = [_files objectAtIndex: row];
	
	[[cell detailTextLabel] setText: [[filename lastPathComponent] stringByDeletingPathExtension]];
	[[cell detailTextLabel] setTextColor:[UIColor grayColor]];
	
	if(_showimage && isFileImage(filename) ) {
		[cell.imageView setContentMode:UIViewContentModeCenter];
		cell.imageView.layer.borderColor = [[UIColor colorWithRed:0.95f green:0.95f blue:0.95f alpha:1.0f]CGColor];
		cell.imageView.layer.borderWidth = 1.0f;
		UIImage *img = [UIImage imageWithContentsOfFile:filename];
		CGSize itemSize = CGSizeMake(38, 38);
		UIGraphicsBeginImageContext(itemSize);
		CGRect imageRect = CGRectMake(0.0, 0.0, itemSize.width, itemSize.height);
		[img drawInRect:imageRect];
		cell.imageView.image = UIGraphicsGetImageFromCurrentImageContext();
		UIGraphicsEndImageContext();
		cell.imageView.layer.masksToBounds = YES;
		cell.imageView.layer.cornerRadius = 5.0f;
	}
	return cell;
}

- (void) tableView: (UITableView*)tableView didSelectRowAtIndexPath: (NSIndexPath*)indexPath
{
	_currentfile = [_files[indexPath.row]retain];
	[_delegate fileSelected:self selectedfile:_currentfile];
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
	return [_files count];
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
	return _title;
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView{
	return 1;
}

- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section {
	return 30.0f;
}

-(void) dealloc {
	[_suffixs release];
	[_paths release];
	[_currentfile release];
	[_title release];
	[_tableView release];
	[_files release];
	
	[super dealloc];
}


@end
