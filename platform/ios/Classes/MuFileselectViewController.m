//
//  MuFileselectViewController.m
//  MxPDF
//
//  Created by 曾亮 on 2016/12/6.
//  Copyright © 2016年 Artifex Software, Inc. All rights reserved.
//

#import "MuFileselectViewController.h"
#import "NTKODsNormalFile.h"
#import "NTKOPfxFile.h"
#import "NTKOEspFile.h"

BOOL isFileImage(NSString *file);
BOOL isFilePfx(NSString *file);
BOOL fileIsInPath(NSArray *paths, NSString *file);
BOOL fileHasSuffixs(NSArray *suffixs, NSString *file);

@interface MuFileselectViewController ()<UITableViewDataSource, UITableViewDelegate>
@end

@implementation MuFileselectViewController {
	FileSelectedBlock _fileselblock;
	id<MuFileSelectViewDelegate> _fileselDelegate;
	
	NSMutableArray<NSArray*>  *_dsArray;
	NSMutableArray<NSString*> *_headers;
	
	NSIndexPath *_selIndexpath;
	NSString *_title;
	IBOutlet UITableView *_table;
}

- (instancetype) initWithDatasource:(NSArray<id<NTKOTableDs>>*)ds Title:(NSString*)title FileSelDelegate:(id<MuFileSelectViewDelegate>)delegate FileSelBlock:(FileSelectedBlock)block
{
	self = [super init];
	if(self) {
		_dsArray = [[NSMutableArray alloc]initWithObjects:ds, nil];
		if(!title) {
			_headers = [[NSMutableArray alloc]initWithObjects:@"section 1", nil];
		}
		_fileselDelegate = delegate; // protecting for retain cycle, use assign
		_fileselblock = Block_copy(block);
		_backViewController = nil;
	}
	return self;
}

- (void) addTableSection:(NSArray<id<NTKOTableDs>>*)sectionDs Title:(NSString*)title {
	[_dsArray addObject:sectionDs];
	[_headers addObject:title];
	[_table reloadData];
}

- (void)dealloc {
	NSLog(@"%@ was dealloced", [self class]);
	if(_fileselblock) _Block_release(_fileselblock);
	// delegate is set as assign, should not be release.
	// if(_delegate) [_delegate release];
	[_dsArray release];
	[_headers release];
	[_table release];
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
	self.view.layer.borderWidth = 0.5f;
	self.view.layer.borderColor = [[UIColor colorWithRed:0.7f green:0.7f blue:0.7f alpha:1.0f]CGColor];
    // Do any additional setup after loading the view from its nib.
	
	self.navigationItem.title = _title;
//	self.navigationItem.leftBarButtonItem = [[[UIBarButtonItem alloc] initWithTitle:@"goback" style:UIBarButtonItemStylePlain target:self action:@selector(onBack:)]autorelease];
	[super viewDidLoad];
}

- (void) onBack:(id)sender {
	if(_backViewController)
		[self.navigationController popToViewController:_backViewController animated:YES];
	else
		[self.navigationController popViewControllerAnimated:YES];
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
		[_fileselDelegate fileSelOkay:self Selected:
		 [[_dsArray objectAtIndex:_selIndexpath.section] objectAtIndex:_selIndexpath.row]];
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
-  (UITableViewCell*) tableView: (UITableView*)tableView cellForRowAtIndexPath: (NSIndexPath*)indexPath
{
	if(!_dsArray || _dsArray.count<=indexPath.section) return nil;
	NSArray *tmpds = [_dsArray objectAtIndex:indexPath.section];
	NSLog(@"section %ld, row %ld", (long)indexPath.section, (long)indexPath.row);
	if(tmpds.count <= indexPath.row) return nil;
	id<NTKOTableDs>ds = [tmpds objectAtIndex:indexPath.row];

	UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier: @"filecell"];
	if (!cell)
		cell = [[[UITableViewCell alloc] initWithStyle: UITableViewCellStyleSubtitle reuseIdentifier: @"filecell"] autorelease];
	
	cell.textLabel.text = ds.title;
	cell.detailTextLabel.text = ds.describe;
	
	UIImage *img = ds.image;
	if(img) {
		CGSize itemSize = CGSizeMake(38, 38);
		UIGraphicsBeginImageContext(itemSize);
		CGRect imageRect = CGRectMake(0.0, 0.0, itemSize.width, itemSize.height);
		[img drawInRect:imageRect];
		cell.imageView.image = UIGraphicsGetImageFromCurrentImageContext();
		UIGraphicsEndImageContext();
		cell.imageView.layer.masksToBounds = YES;
		cell.imageView.layer.cornerRadius = 5.0f;
	}
	else {
		cell.imageView.image = nil;
	}
	return cell;
}

- (void) tableView: (UITableView*)tableView didSelectRowAtIndexPath: (NSIndexPath*)indexPath
{
	id<NTKOTableDs> ds = [[_dsArray objectAtIndex:indexPath.section] objectAtIndex:indexPath.row];
	
	if(_selIndexpath) [_selIndexpath release];
	_selIndexpath = [indexPath retain];

	BOOL close = NO;
	if(_fileselblock)
		close = _fileselblock(self, ds, indexPath);
	if(_fileselDelegate)
		close = close || [_fileselDelegate fileSelected:self Selected:ds selectdIndexpath:indexPath];
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
	return _dsArray.count;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
	return [_dsArray objectAtIndex:section].count;
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section
{
	return [_headers objectAtIndex:section];
}

- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section
{
	return 0==section ? 0 : 25.0f;
}

+ (void) pushDefaultImageSelect: (UIViewController*)parentVC SelDelegate:(id<MuFileSelectViewDelegate>)delegate SelBlock:(FileSelectedBlock)block {

	MuFileselectViewController *vc = [[MuFileselectViewController defaultImageViewController: delegate SelBlock:block]autorelease];
	[parentVC.navigationController pushViewController:vc animated:true];
}

+ (void)showDefaultImageSelect: (UIViewController*)parentVC SelDelegate:(id<MuFileSelectViewDelegate>)delegate SelBlock:(FileSelectedBlock)block
{
	MuFileselectViewController *vc = [MuFileselectViewController defaultImageViewController: delegate SelBlock:block];
	
	vc.modalPresentationStyle = UIModalPresentationFormSheet;
	vc.modalTransitionStyle = UIModalTransitionStyleCoverVertical;
	[parentVC presentViewController:vc animated:NO completion:nil];
	
	[vc release];
}

+ (void) pushDefaultPfxSelect: (UIViewController*)parentVC fileselectedDelegate:(id<MuFileSelectViewDelegate>)delegate fileselectedblock:(FileSelectedBlock)block;
{
	MuFileselectViewController *vc = [[MuFileselectViewController defaultPfxViewController:delegate SelBlock:block]autorelease];
	[parentVC.navigationController pushViewController:vc animated:true];
}

+ (void)showDefaultPfxSelect: (UIViewController*)parentVC SelDelegate:(id<MuFileSelectViewDelegate>)delegate SelBlock:(FileSelectedBlock)block
{
	MuFileselectViewController *vc = [MuFileselectViewController defaultPfxViewController: delegate SelBlock:block];
	
	vc.modalPresentationStyle = UIModalPresentationFormSheet;
	vc.modalTransitionStyle = UIModalTransitionStyleCoverVertical;
	[parentVC presentViewController:vc animated:NO completion:nil];
	
	[vc release];
}

+ (NSArray<id<NTKOTableDs>>*) filesDs:(NSArray *)paths Suffixs:(NSArray*) suffixs TheClass:(Class)class  {
	
	NSMutableArray<id<NTKOTableDs>> *files = [[NSMutableArray<id<NTKOTableDs>> alloc]initWithCapacity:3];
	NSFileManager *fileman = [NSFileManager defaultManager];
	NSString *docdir = [NSString stringWithFormat: @"%@/Documents", NSHomeDirectory()];
	
	NSDirectoryEnumerator *direnum = [fileman enumeratorAtPath:docdir];
	NSString *file;
	BOOL isdir;
	id<NTKOTableDs> one = nil;
	
	while ( (file = [direnum nextObject]) ) {
		NSString *filepath = [docdir stringByAppendingPathComponent:file];
		if ([fileman fileExistsAtPath:filepath isDirectory:&isdir] && !isdir) {
			if(fileIsInPath(paths, file) && fileHasSuffixs(suffixs, file)) {
				one = [[[class alloc]initWithFile:filepath]autorelease];
				[files addObject:one];
			}
		}
	}
	return files;
}

+ (NSArray*) defaultImages {
	NSArray *suffixs = [NSArray arrayWithObjects:@".bmp",@".jpeg",@".jpg",@".bmp",@".png", @".gif", nil];
	NSArray *paths = [NSArray arrayWithObjects:@"_imagefiles", nil];
	NSArray<id<NTKOTableDs>>* dss = [MuFileselectViewController filesDs:paths Suffixs:suffixs TheClass:[NTKODsNormalFile class]];
	return dss;
}

+ (instancetype) defaultImageViewController: (id<MuFileSelectViewDelegate>)delegate SelBlock:(FileSelectedBlock)block
{
	NSArray<id<NTKOTableDs>>* dss = [[MuFileselectViewController defaultImages]autorelease];
	MuFileselectViewController *vc = [[MuFileselectViewController alloc]initWithDatasource:dss Title:nil FileSelDelegate:delegate FileSelBlock:block];
	vc.title = @"image file(*.bmp,*.jpg,*.png,*.gif...)";
	return vc;
}

+ (NSArray*)defaultPfxs {
	NSArray *paths = [NSArray arrayWithObjects:@"_pfxfiles", nil];
	NSArray *suffixs = [NSArray arrayWithObjects:@".pfx", nil];
	NSArray<id<NTKOTableDs>>* dss = [MuFileselectViewController filesDs:paths Suffixs:suffixs TheClass:[NTKOPfxFile class]];
	return dss;
}

+ (NSArray*) defaultSigns {
	NSArray *paths = [NSArray arrayWithObjects:@"_espfiles", nil];
	NSArray *suffixs = [NSArray arrayWithObjects:@".esp", nil];
	NSArray<id<NTKOTableDs>>* dss = [MuFileselectViewController filesDs:paths Suffixs:suffixs TheClass:[NTKOEspFile class]];
	return dss;
}

+ (instancetype) defaultPfxViewController: (id<MuFileSelectViewDelegate>)delegate SelBlock:(FileSelectedBlock)block {
	NSArray<id<NTKOTableDs>>* dss = [[MuFileselectViewController defaultPfxs]autorelease];
	MuFileselectViewController *vc = [[MuFileselectViewController alloc]initWithDatasource:dss Title:nil FileSelDelegate:delegate FileSelBlock:block];
	vc.title = @"pfx file(*.pfx)";
	return vc;
}

@end


