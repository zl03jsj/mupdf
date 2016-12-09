//
//  MuFileselectViewController.m
//  MxPDF
//
//  Created by 曾亮 on 2016/12/6.
//  Copyright © 2016年 Artifex Software, Inc. All rights reserved.
//

#import "MuFileselectViewController.h"

BOOL isFileImage(NSString *file);
BOOL isFilePfx(NSString *file);
BOOL fileIsInPath(NSArray *paths, NSString *file);
BOOL fileHasSuffixs(NSArray *suffixs, NSString *file);

@interface MuFileselectViewController ()<UITableViewDataSource, UITableViewDelegate>
@end

@implementation MuFileselectViewController {
	FileSelectedBlock _fileselblock;
	id<MuFileSelectViewDelegate> _fileselDelegate;
	
	NSArray *_usersection;
	NSString *_usersectionTitle;
	NSArray *_paths;
	NSArray *_suffixs;
	BOOL _isshowimage;
	BOOL _closeOnFileSelected;
	
	NSString *_selectedfile;
	NSMutableArray *_files;

	NSString *_title;
	
	IBOutlet UITableView *_table;
	IBOutlet UINavigationItem *_navigationitem;
}

- (void)initFiles {
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
}

- (instancetype)initWithChoices:(NSString*)title userSection:(NSArray *)section userSectionTitle:(NSString*)sectionTitle filesuffixs:(NSArray*)suffixs filepaths:(NSArray*)paths isshowimage:(BOOL)isshow fileselectedDelegate:(id<MuFileSelectViewDelegate>)delegate fileselectedblock:(FileSelectedBlock)fileselblock
{
	self = [super init];
	// if(fileselblock) _fileselblock = Block_copy(fileselblock);
	_fileselectedBlock = nil;
	_fileselDelegate = delegate;
	
	_usersection = [section retain];
	_usersectionTitle = [sectionTitle copy];
	
	_title = [title copy];
	
	_paths = paths?[paths retain]:nil;
	_suffixs = suffixs?[suffixs retain]:nil;
	_isshowimage = isshow;
	_closeOnFileSelected = NO;
	
	_selectedfile = nil;
	_files = nil;
	
	[self initFiles];
	return self;
}

- (void)dealloc {
	if(_fileselblock) _Block_release(_fileselblock);
	if(_usersection) [_usersection release];
	if(_usersectionTitle) [_usersectionTitle release];
	if(_paths) [_paths release];
	if(_suffixs) [_suffixs release];
	if(_selectedfile) [_selectedfile release];
	if(_files) [_files release];
	if(_title) [_title release];

	// delegate property is set as assign, should not be release.
	// if(_delegate) [_delegate release];
	[_table release];
	[_navigationitem release];
	[super dealloc];
}

- (void) viewWillAppear:(BOOL)animated {
	[super viewWillAppear:animated];
}

- (void) viewWillDisappear:(BOOL)animated {
	[super viewWillDisappear:animated];
}

- (void)viewDidLoad {
    [super viewDidLoad];
	
	_table.delegate = self;
	_table.dataSource = self;
	
	// set border style
	self.view.layer.cornerRadius = 4.0f;
	self.view.layer.borderColor = [[UIColor colorWithRed:0.7f green:0.7f blue:0.7f alpha:1.0f]CGColor];
	self.view.layer.borderWidth = 1.5f;
    // Do any additional setup after loading the view from its nib.
	
	_navigationitem.title = _title;
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)closeTaped:(id)sender {
	if(_fileselDelegate)
	   [_fileselDelegate fileSelCancel:self];
	[self dismissViewControllerAnimated:YES completion:nil];
}

- (IBAction)okayTaped:(id)sender {
	if(_fileselDelegate)
		[_fileselDelegate fileSelOkay:self selectedfile:_selectedfile];
	[self dismissViewControllerAnimated:YES completion:nil];
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

#pragma mark -------- table view delegates and datasource!!!
- (UITableViewCell*) tableView: (UITableView*)tableView cellForRowAtIndexPath: (NSIndexPath*)indexPath
{
	NSString *filename = nil;//[_files objectAtIndex: row];
	
	if(indexPath.section==0) filename = _files[indexPath.row];
	if(indexPath.section==1) filename = _usersection[indexPath.row];
	if(!filename) return nil;
	
	UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier: @"filecell"];
	if (!cell)
		cell = [[[UITableViewCell alloc] initWithStyle: UITableViewCellStyleSubtitle reuseIdentifier: @"filecell"] autorelease];
	
	[[cell detailTextLabel] setText: [[filename lastPathComponent] stringByDeletingPathExtension]];
	[[cell detailTextLabel] setTextColor:[UIColor grayColor]];
	
	if(_isshowimage && isFileImage(filename) ) {
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
	_selectedfile = [(indexPath.section==0?_files[indexPath.row]:_usersection[indexPath.row])copy];
	BOOL close = NO;
	if(_fileselblock)
		close = _fileselblock(self, _selectedfile, indexPath);
	if(_fileselDelegate)
		close = close || [_fileselDelegate fileSelected:self selectedfile:_selectedfile selectdIndexpath:indexPath];
	if(close)
		[self dismissViewControllerAnimated:YES completion:nil];
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
	return section==0?(_files?_files.count:0):(_usersection?_usersection.count:0);
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section
{
	return section==1 ? _usersectionTitle:nil;
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView{
	NSInteger sections = 0;
	if(_files && _files.count) sections++;
	if(_usersection && _usersection.count) sections++;
	return sections;
}

- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section
{
	return 0==section ? 0 : 25.0f;
}

+ (void)showDefaultImageSelect: (UIViewController*)parentVC fileselectedDelegate:(id<MuFileSelectViewDelegate>)delegate fileselectedblock:(FileSelectedBlock)fileselblock
{
	NSArray *imagefilesuffixs = [NSArray arrayWithObjects:@".bmp",@".jpeg",@".jpg",@".bmp",@".png", @".gif", nil];
	NSArray *imagefilepaths = [NSArray arrayWithObjects:@"_imagefiles", nil];
	NSString *viewtitle = @"image file(*.bmp,*.jpg,*.png,*.gif)";
	
	MuFileselectViewController *vc = [[MuFileselectViewController alloc]initWithChoices:viewtitle userSection:nil userSectionTitle:nil filesuffixs:imagefilesuffixs filepaths:imagefilepaths isshowimage:YES fileselectedDelegate:delegate fileselectedblock:fileselblock];
	vc.modalPresentationStyle = UIModalPresentationFormSheet;
	vc.modalTransitionStyle = UIModalTransitionStyleCoverVertical;
	[parentVC presentViewController:vc animated:NO completion:nil];
	[vc release];
}

+ (void)showDefaultPfxSelect: (UIViewController*)parentVC fileselectedDelegate:(id<MuFileSelectViewDelegate>)delegate fileselectedblock:(FileSelectedBlock)fileselblock;
{
	NSArray *pfxfilepaths = [NSArray arrayWithObjects:@"_pfxfiles", nil];
	NSArray *pfxfilesuffixs = [NSArray arrayWithObjects:@".pfx", nil];
	NSArray *pfxsection = [NSArray arrayWithObjects:@"use bluetooth key device", nil];
	NSString *viewtitle = @"pfx file(*.pfx)";
	NSString *sectiontitle = @"other sign device";
	
	MuFileselectViewController *vc = [[MuFileselectViewController alloc]initWithChoices:viewtitle userSection:pfxsection userSectionTitle:sectiontitle filesuffixs:pfxfilesuffixs filepaths:pfxfilepaths isshowimage:NO fileselectedDelegate:delegate fileselectedblock:nil];

	vc.modalPresentationStyle = UIModalPresentationFullScreen;
	vc.modalTransitionStyle = UIModalTransitionStyleCoverVertical;
	[parentVC presentViewController:vc animated:NO completion:nil];
}
@end

BOOL isFileImage(NSString *file)
{
	NSString const* image_suffix[] = {@".bmp",@".jpeg",@".jpg",@".bmp",@".png", @".gif"};
	for(int i=0; i<sizeof(image_suffix)/sizeof(NSString*); i++)
		if([[file lowercaseString] hasSuffix:(NSString*)image_suffix[i]])
			return YES;
	return NO;
}

BOOL isFilePfx(NSString *file)
{
	return [[file lowercaseString]hasSuffix:@".pfx"];
}

BOOL fileIsInPath(NSArray *paths, NSString *file)
{
	for(NSString *path in paths) {
		if([[file lowercaseString ]hasPrefix:[path lowercaseString]])
			return YES;
	}
	return NO;
}

BOOL fileHasSuffixs(NSArray *suffixs, NSString *file)
{
	for(NSString *suffix in suffixs) {
		if([[file lowercaseString] hasSuffix:[suffix lowercaseString]])
			return YES;
	}
	return NO;
}
