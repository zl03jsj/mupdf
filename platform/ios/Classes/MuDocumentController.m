#import "common.h"

#import "MuPageViewNormal.h"
#import "MuPageViewReflow.h"
#import "MuDocumentController.h"
#import "MuTextFieldController.h"
#import "MuChoiceFieldController.h"
#import "MuFileselectViewController.h"
#import "MuPrintPageRenderer.h"
#import "NTKOViewController.h"
#import "NTKODsNormalFile.h"
#import "NTKOEspFile.h"
#import "NTKOPfxFile.h"

#define GAP 20
#define INDICATOR_Y -44-24
#define SLIDER_W (width - GAP - 24)
#define SEARCH_W (width - GAP - 170)
#define MIN_SCALE (1.0)
#define MAX_SCALE (5.0)

static NSString *const AlertTitle = @"Save Document?";
// Correct functioning of the app relies on CloseAlertMessage and ShareAlertMessage differing
static NSString *const CloseAlertMessage = @"Changes have been made to the document that will be lost if not saved";
static NSString *const ShareAlertMessage = @"Your changes will not be shared unless the document is first saved";

static void flattenOutline(NSMutableArray *titles, NSMutableArray *pages, fz_outline *outline, int level)
{
	char indent[8*4+1];
	if (level > 8)
		level = 8;
	memset(indent, ' ', level * 4);
	indent[level * 4] = 0;
	while (outline)
	{
		int page = outline->page;
		if (page >= 0 && outline->title)
		{
			NSString *title = @(outline->title);
			[titles addObject: [NSString stringWithFormat: @"%s%@", indent, title]];
			[pages addObject: @(page)];
		}
		flattenOutline(titles, pages, outline->down, level + 1);
		outline = outline->next;
	}
}

static char *tmp_path(const char *path)
{
	int f;
	char *buf = (char*)malloc(strlen(path) + 6 + 1);
	if (!buf)
		return NULL;

	strcpy(buf, path);
	strcat(buf, "XXXXXX");

	f = mkstemp(buf);

	if (f >= 0)
	{
		close(f);
		return buf;
	}
	else
	{
		free(buf);
		return NULL;
	}
}

static void saveDoc(const char *current_path, fz_document *doc)
{
	char *tmp;
	pdf_document *idoc = pdf_specifics(ctx, doc);
	pdf_write_options opts = { 0 };

	opts.do_incremental = 1;

	if (!idoc)
		return;

	tmp = tmp_path(current_path);
	if (tmp)
	{
		int written = 0;

		fz_var(written);
		fz_try(ctx)
		{
			FILE *fin = fopen(current_path, "rb");
			FILE *fout = fopen(tmp, "wb");
			char buf[256];
			size_t n;
			int err = 1;

			if (fin && fout)
			{
				while ((n = fread(buf, 1, sizeof(buf), fin)) > 0)
					fwrite(buf, 1, n, fout);
				err = (ferror(fin) || ferror(fout));
			}

			if (fin)
				fclose(fin);
			if (fout)
				fclose(fout);

			if (!err)
			{
				pdf_save_document(ctx, idoc, tmp, &opts);
				written = 1;
			}
		}
		fz_catch(ctx)
		{
			written = 0;
		}

		if (written)
		{
			rename(tmp, current_path);
		}

		free(tmp);
	}
}

static z_pdf_sign_appearance* createAppearanceWithNSData(NSData *imgdata, CGRect rect) {
	z_pdf_sign_appearance *app = NULL;
	fz_image *image = NULL;
	unsigned char *data = NULL;
	fz_try(ctx) {
		data = fz_malloc(ctx, imgdata.length);
		int size = (int)imgdata.length;
		
		[imgdata getBytes:data length:size];
		
		fz_rect r = {rect.origin.x, rect.origin.y, rect.origin.x + rect.size.width,
			rect.origin.y + rect.size.height};
		
		// the data was not copyed
		// can not be free
		image = fz_new_image_from_data(ctx, data, size);
		if(image) data = NULL;
		app = z_pdf_new_sign_appearance_with_image(ctx, image, r, NULL);
	}
	fz_always(ctx) {
		if(data) fz_free(ctx, data);
		if(image) fz_drop_image(ctx, image);
	}
	fz_catch(ctx) {
		NSLog(@"create sign appearance faild.\nerror message:%s.", ctx->error->message);
	}
	return app;
}

#if 0
static z_pdf_sign_appearance* createAppearanceWithImagefile(NSString *imagefile, CGRect rect) {
	if(!imagefile) return NULL;
	z_pdf_sign_appearance *app = NULL;
	NSData *imgdata = [NSData dataWithContentsOfFile:imagefile];
	if(!imgdata) return NULL;
	
	fz_try(ctx)
		app = createAppearanceWithNSData(imgdata, rect);
	fz_catch(ctx)
		fz_rethrow(ctx);
	return app;
}
#endif

static z_pdf_sign_appearance *createAppearanceWithPointArrayList(z_fpoint_arraylist *al, CGRect rect) {
	if(!al) return NULL;
	z_pdf_sign_appearance *app = NULL;
	
	fz_try(ctx) {
		fz_rect r = {rect.origin.x, rect.origin.y, rect.origin.x + rect.size.width,
			rect.origin.y + rect.size.height};
		app = z_pdf_new_sign_appearance_with_paths(ctx, al, r, NULL);
	}
	fz_always(ctx) {
	}
	fz_catch(ctx) {
		NSLog(@"create sign appearance faild.\nerror message:%s.", ctx->error->message);
	}
	return app;
}

@implementation MuDocumentController
{
	fz_document *doc;
	MuDocRef *docRef;
	NSString *key;
	NSString *_filePath;
	BOOL reflowMode;
	MuOutlineController *outline;
	UIScrollView *canvas;
	UILabel *indicator;
	UISlider *slider;
	UISearchBar *searchBar;
	UIBarButtonItem *nextButton, *prevButton, *cancelButton, *searchButton, *outlineButton, *linkButton;
	UIBarButtonItem *moreButton;
	UIBarButtonItem *shareButton, *printButton, *annotButton;
	UIBarButtonItem *highlightButton, *underlineButton, *strikeoutButton;
	UIBarButtonItem *inkButton;
	UIBarButtonItem *tickButton;
	UIBarButtonItem *deleteButton;
	UIBarButtonItem *reflowButton;
	UIBarButtonItem *backButton;
	UIBarButtonItem *sliderWrapper;
	
	// add by zl [2016/11/16 5:25]
	// add signature with image appearance!
	UIBarButtonItem *handsignButton;
	UIBarButtonItem *signButton;
	UIBarButtonItem *nextstepButton;
	z_pdf_sign_appearance *_app;
	z_device *_device;
	Signstep _signstep;
	
	NTKOViewController *_loginVc;
	MuFileselectViewController *_svrsepSelVc;
	NTKOEspFile *_espFile;
	ntko_sign_info *_signinfo;
	UIActivityIndicatorView *_holdingView;
	/////////////////////////////////////////
	
	int barmode;
	int searchPage;
	int cancelSearch;
	int showLinks;
	int width; // current screen size
	int height;
	int current; // currently visible page
	int scroll_animating; // stop view updates during scrolling animations
	float scale; // scale applied to views (only used in reflow mode)
	BOOL _isRotating;
}

- (instancetype) initWithFilename: (NSString*)filename path:(NSString *)cstr document: (MuDocRef *)aDoc
{
	self = [super initWithNibName:nil bundle:nil];
	if (!self)
		return nil;

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 70000
	if ([self respondsToSelector:@selector(automaticallyAdjustsScrollViewInsets)])
		self.automaticallyAdjustsScrollViewInsets = NO;
#endif
	key = [filename copy];
	docRef = [aDoc retain];
	doc = docRef->doc;
	_filePath = [cstr copy];

	//  this will be created right before the outline is shown
	outline = nil;
	_app = NULL;
	_device = NULL;

	_loginVc = nil;
	_svrsepSelVc = nil;
	_espFile = nil;
	_signinfo = NULL;
	dispatch_sync(queue, ^{});

	return self;
}

- (UIBarButtonItem *) newResourceBasedButton:(NSString *)resource withAction:(SEL)selector
{
	if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
	{
		return [[UIBarButtonItem alloc] initWithImage:[UIImage imageWithContentsOfFile:[[NSBundle mainBundle] pathForResource:resource ofType:@"png"]] style:UIBarButtonItemStylePlain target:self action:selector];
	}
	else
	{
		UIView *buttonView;
		BOOL iOS7Style = (([UIDevice currentDevice].systemVersion).floatValue >= 7.0f);
		UIButton *button = [UIButton buttonWithType:iOS7Style ? UIButtonTypeSystem : UIButtonTypeCustom];
		[button setImage:[UIImage imageNamed:resource] forState:UIControlStateNormal];
		[button addTarget:self action:selector forControlEvents:UIControlEventTouchUpInside];
		[button sizeToFit];
		buttonView = button;

		return [[UIBarButtonItem alloc] initWithCustomView:buttonView];
	}
}

- (void) addMainMenuButtons
{
	NSMutableArray *array = [NSMutableArray arrayWithCapacity:3];
	[array addObject:moreButton];
	[array addObject:searchButton];
	if (outlineButton)
		[array addObject:outlineButton];
	[array addObject:reflowButton];
	[array addObject:linkButton];
	self.navigationItem.rightBarButtonItems = array ;
	self.navigationItem.leftBarButtonItem = backButton;
}

- (void) loadView
{
	[[NSUserDefaults standardUserDefaults] setObject: key forKey: @"OpenDocumentKey"];

	current = (int)[[NSUserDefaults standardUserDefaults] integerForKey: key];
	if (current < 0 || current >= fz_count_pages(ctx, doc))
		current = 0;

	UIView *view = [[UIView alloc] initWithFrame: CGRectZero];
	view.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
	[view setAutoresizesSubviews: YES];
	view.backgroundColor = [UIColor grayColor];

	canvas = [[UIScrollView alloc] initWithFrame: CGRectMake(0,0,GAP,0)];
	canvas.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
	[canvas setPagingEnabled: YES];
	[canvas setShowsHorizontalScrollIndicator: NO];
	[canvas setShowsVerticalScrollIndicator: NO];
	canvas.delegate = self;

	UITapGestureRecognizer *tapRecog = [[UITapGestureRecognizer alloc] initWithTarget: self action: @selector(onTap:)];
	tapRecog.delegate = self;
	[canvas addGestureRecognizer: tapRecog];
	[tapRecog release];
	
	// In reflow mode, we need to track pinch gestures on the canvas and pass
	// the scale changes to the subviews.
	UIPinchGestureRecognizer *pinchRecog = [[UIPinchGestureRecognizer alloc] initWithTarget:self action:@selector(onPinch:)];
	pinchRecog.delegate = self;
	[canvas addGestureRecognizer:pinchRecog];
	[pinchRecog release];

	scale = 1.0;

	scroll_animating = NO;

	indicator = [[UILabel alloc] initWithFrame: CGRectZero];
	indicator.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleRightMargin | UIViewAutoresizingFlexibleTopMargin;
	indicator.text = @"0000 of 9999";
	[indicator sizeToFit];
	indicator.center = CGPointMake(0, INDICATOR_Y);
	indicator.textAlignment = NSTextAlignmentCenter;
	indicator.backgroundColor = [[UIColor blackColor] colorWithAlphaComponent: 0.5];
	indicator.textColor = [UIColor whiteColor];

	[view addSubview: canvas];
	[view addSubview: indicator];

	slider = [[UISlider alloc] initWithFrame: CGRectZero];
	slider.minimumValue = 0;
	slider.maximumValue = fz_count_pages(ctx, doc) - 1;
	[slider addTarget: self action: @selector(onSlide:) forControlEvents: UIControlEventValueChanged];

	if ([UIDevice currentDevice].systemVersion.floatValue < 7.0)
	{
		sliderWrapper = [[UIBarButtonItem alloc] initWithCustomView: slider];

		self.toolbarItems = @[sliderWrapper];
	}

	// Set up the buttons on the navigation and search bar

	fz_outline *outlineRoot = NULL;
	fz_try(ctx)
		outlineRoot = fz_load_outline(ctx, doc);
	fz_catch(ctx)
		outlineRoot = NULL;
	if (outlineRoot)
	{
		//  only show the outline button if there is an outline
		outlineButton = [self newResourceBasedButton:@"ic_list" withAction:@selector(onShowOutline:)];
		fz_drop_outline(ctx, outlineRoot);
	}

	linkButton = [self newResourceBasedButton:@"ic_link" withAction:@selector(onToggleLinks:)];
	cancelButton = [self newResourceBasedButton:@"ic_cancel" withAction:@selector(onCancel:)];
	searchButton = [self newResourceBasedButton:@"ic_magnifying_glass" withAction:@selector(onShowSearch:)];
	prevButton = [self newResourceBasedButton:@"ic_arrow_left" withAction:@selector(onSearchPrev:)];
	nextButton = [self newResourceBasedButton:@"ic_arrow_right" withAction:@selector(onSearchNext:)];
	reflowButton = [self newResourceBasedButton:@"ic_reflow" withAction:@selector(onToggleReflow:)];
	moreButton = [self newResourceBasedButton:@"ic_more" withAction:@selector(onMore:)];
	annotButton = [self newResourceBasedButton:@"ic_annotation" withAction:@selector(onAnnot:)];
	shareButton = [self newResourceBasedButton:@"ic_share" withAction:@selector(onShare:)];
	printButton = [self newResourceBasedButton:@"ic_print" withAction:@selector(onPrint:)];
	highlightButton = [self newResourceBasedButton:@"ic_highlight" withAction:@selector(onHighlight:)];
	underlineButton = [self newResourceBasedButton:@"ic_underline" withAction:@selector(onUnderline:)];
	strikeoutButton = [self newResourceBasedButton:@"ic_strike" withAction:@selector(onStrikeout:)];
	inkButton = [self newResourceBasedButton:@"ic_pen" withAction:@selector(onInk:)];
	tickButton = [self newResourceBasedButton:@"ic_check" withAction:@selector(onTick:)];
	deleteButton = [self newResourceBasedButton:@"ic_trash" withAction:@selector(onDelete:)];
	
	// add signature [2016/11/16 17:33] by zl
	handsignButton = [self newResourceBasedButton:@"ic_signature" withAction:@selector(onHandsign:)];
	signButton = [self newResourceBasedButton:@"ic_stamp" withAction:@selector(onSign:)];
	nextstepButton = [self newResourceBasedButton:@"ic_next" withAction:@selector(onNextstep:)];
	if( !pdf_signatures_supported(ctx) ) {
		handsignButton.enabled = NO;
		signButton.enabled = NO;
		nextstepButton.enabled = NO;		
	}
	
	searchBar = [[UISearchBar alloc] initWithFrame: CGRectMake(0,0,50,32)];
	backButton = [self newResourceBasedButton:@"ic_arrow_left" withAction:@selector(onBack:)];
	searchBar.placeholder = @"Search";
	searchBar.delegate = self;

	[prevButton setEnabled: NO];
	[nextButton setEnabled: NO];

	[self addMainMenuButtons];
 
	// TODO: add activityindicator to search bar

	self.view = view;
	[view release];
}

- (void) dealloc
{
	NSLog(@"%@ was dealloced", [self class]);
	[docRef release]; docRef = nil; doc = NULL;
	[indicator release]; indicator = nil;
	[slider release]; slider = nil;
	[sliderWrapper release]; sliderWrapper = nil;
	[reflowButton release]; reflowButton = nil;
	[backButton release]; backButton = nil;
	[moreButton release]; moreButton = nil;
	[searchBar release]; searchBar = nil;
	[outlineButton release]; outlineButton = nil;
	[linkButton release]; linkButton = nil;
	[searchButton release]; searchButton = nil;
	[cancelButton release]; cancelButton = nil;
	[prevButton release]; prevButton = nil;
	[nextButton release]; nextButton = nil;
	[shareButton release]; shareButton = nil;
	[printButton release]; printButton = nil;
	[annotButton release]; annotButton = nil;
	[highlightButton release]; highlightButton = nil;
	[underlineButton release]; underlineButton = nil;
	[strikeoutButton release]; strikeoutButton = nil;
	[inkButton release]; inkButton = nil;
	[tickButton release]; tickButton = nil;
	[signButton release]; signButton = nil;
	[handsignButton release]; handsignButton = nil;
	[nextstepButton release]; nextstepButton = nil;
	[deleteButton release]; deleteButton = nil;
	[canvas release]; canvas = nil;
	[_filePath release]; _filePath = NULL;

	[outline release];
	if(_app) z_pdf_drop_sign_appreance(ctx, _app);
	if(_device) z_drop_device(ctx, _device);
	[key release];
	
	
	if(_loginVc) [_loginVc release];
	
	[super dealloc];
}

- (void) viewWillAppear: (BOOL)animated
{
	NSLog(@"viewWillAppear");
	[super viewWillAppear:animated];
	self.title = key.lastPathComponent;

	slider.value = current;

	if ([UIDevice currentDevice].systemVersion.floatValue >= 7.0)
		[self.navigationController.toolbar addSubview:slider];

	indicator.text = [NSString stringWithFormat: @" %d of %d ", current+1, fz_count_pages(ctx, doc)];

	[self.navigationController setToolbarHidden: NO animated: animated];
}

- (void) viewWillLayoutSubviews
{
	CGSize size = canvas.frame.size;
	int max_width = fz_max(width, size.width);

	width = size.width;
	height = size.height;

	canvas.contentInset = UIEdgeInsetsZero;
	canvas.contentSize = CGSizeMake(fz_count_pages(ctx, doc) * width, height);
	canvas.contentOffset = CGPointMake(current * width, 0);

	[sliderWrapper setWidth: SLIDER_W];
	searchBar.frame = CGRectMake(0,0,SEARCH_W,32);
	if ([UIDevice currentDevice].systemVersion.floatValue >= 7.0)
	{
		CGRect r = self.navigationController.toolbar.frame;
		r.origin.x = 0;
		r.origin.y = 0;
		slider.frame = r;
	}

	[self.navigationController.toolbar setNeedsLayout]; // force layout!
	// use max_width so we don't clamp the content offset too early during animation
	canvas.contentSize = CGSizeMake(fz_count_pages(ctx, doc) * max_width, height);
	canvas.contentOffset = CGPointMake(current * width, 0);

	for (UIView<MuPageView> *view in canvas.subviews) {
		if (view.number == current) {
			view.frame = CGRectMake(view.number * width, 0, width-GAP, height);
			[view willRotate];
		}
	}
}

- (void) viewDidAppear: (BOOL)animated
{
	[super viewDidAppear:animated];
	[self scrollViewDidScroll: canvas];
}

- (void) viewWillDisappear: (BOOL)animated
{
	[super viewWillDisappear:animated];
	if ([UIDevice currentDevice].systemVersion.floatValue >= 7.0)
		[slider removeFromSuperview];

	self.title = @"Resume";
	[[NSUserDefaults standardUserDefaults] removeObjectForKey: @"OpenDocumentKey"];
	[self.navigationController setToolbarHidden: YES animated: animated];
}

- (void) showNavigationBar
{
	// NSLog(@"showNavigationBar");
	if (self.navigationController.navigationBarHidden) {
		[sliderWrapper setWidth: SLIDER_W];
		if ([UIDevice currentDevice].systemVersion.floatValue >= 7.0)
		{
			CGRect r = self.navigationController.toolbar.frame;
			r.origin.x = 0;
			r.origin.y = 0;
			slider.frame = r;
		}
		[self.navigationController setNavigationBarHidden: NO];
		[self.navigationController setToolbarHidden: NO];
		[indicator setHidden: NO];

		[UIView beginAnimations: @"MuNavBar" context: NULL];

		self.navigationController.navigationBar.alpha = 1;
		self.navigationController.toolbar.alpha = 1;
		indicator.alpha = 1;

		[UIView commitAnimations];
	}
}

- (void) hideNavigationBar
{
	NSLog(@"hideNavigationBar");
	if (!self.navigationController.navigationBarHidden) {
		[searchBar resignFirstResponder];

		[UIView beginAnimations: @"MuNavBar" context: NULL];
		[UIView setAnimationDelegate: self];
		[UIView setAnimationDidStopSelector: @selector(onHideNavigationBarFinished)];

		self.navigationController.navigationBar.alpha = 0;
		self.navigationController.toolbar.alpha = 0;
		indicator.alpha = 0;

		[UIView commitAnimations];
	}
}

- (void) onHideNavigationBarFinished
{
	[self.navigationController setNavigationBarHidden: YES];
	[self.navigationController setToolbarHidden: YES];
	[indicator setHidden: YES];
}

- (void) onShowOutline: (id)sender
{
	//  rebuild the outline in case the layout has changed
	fz_outline *root = NULL;
	fz_try(ctx)
		root = fz_load_outline(ctx, doc);
	fz_catch(ctx)
		root = NULL;
	if (root)
	{
		NSMutableArray *titles = [[NSMutableArray alloc] init];
		NSMutableArray *pages = [[NSMutableArray alloc] init];
		flattenOutline(titles, pages, root, 0);
		[outline release];
		if (titles.count)
			outline = [[MuOutlineController alloc] initWithTarget: self titles: titles pages: pages];
		[titles release];
		[pages release];
		fz_drop_outline(ctx, root);
	}

	//  now show it
	[self.navigationController pushViewController: outline animated: YES];
}

- (void) onToggleLinks: (id)sender
{
	showLinks = !showLinks;
	for (UIView<MuPageView> *view in canvas.subviews)
	{
		if (showLinks)
			[view showLinks];
		else
			[view hideLinks];
	}
}

- (void) onToggleReflow: (id)sender
{
	reflowMode = !reflowMode;

	annotButton.enabled = !reflowMode;
	searchButton.enabled = !reflowMode;
	linkButton.enabled = !reflowMode;
	moreButton.enabled = !reflowMode;

	[canvas.subviews makeObjectsPerformSelector:@selector(removeFromSuperview)];
	[self scrollViewDidScroll:canvas];
}

- (void) showMoreMenu
{
	NSMutableArray *rightbuttons = [NSMutableArray arrayWithObjects:printButton, shareButton, nil];
	if (docRef->interactive)
		[rightbuttons insertObject:annotButton atIndex:0];
	self.navigationItem.rightBarButtonItems = rightbuttons;
	self.navigationItem.leftBarButtonItem = cancelButton;

	barmode = BARMODE_MORE;
}

- (void) showAnnotationMenu
{
	self.navigationItem.rightBarButtonItems = @[handsignButton, signButton, inkButton, strikeoutButton, underlineButton, highlightButton];
	self.navigationItem.leftBarButtonItem = cancelButton;

	for (UIView<MuPageView> *view in canvas.subviews)
	{
		if (view.number == current)
			[view deselectAnnotation];
	}

	barmode = BARMODE_ANNOTATION;
}

- (void) showOkMenu
{
	[[self navigationItem] setRightBarButtonItems:[NSArray arrayWithObjects:nextstepButton, nil]];
	[[self navigationItem] setLeftBarButtonItem:cancelButton];
	
	for (UIView<MuPageView> *view in [canvas subviews])
	{
		if ([view number] == current)
			[view deselectAnnotation];
	}
}

- (void) update
{
	for (UIView<MuPageView> *view in canvas.subviews)
		[view update];
}

- (void) onMore: (id)sender
{
	[self showMoreMenu];
}

- (void) onAnnot: (id)sender
{
	[self showAnnotationMenu];
}

- (void) onPrint: (id)sender
{
	UIPrintInteractionController *pic = [UIPrintInteractionController sharedPrintController];
	if (pic) {
		UIPrintInfo *printInfo = [UIPrintInfo printInfo];
		printInfo.outputType = UIPrintInfoOutputGeneral;
		printInfo.jobName = key;
		printInfo.duplex = UIPrintInfoDuplexLongEdge;
		pic.printInfo = printInfo;
		pic.showsPageRange = YES;
		pic.printPageRenderer = [[[MuPrintPageRenderer alloc] initWithDocRef:docRef] autorelease];

		void (^completionHandler)(UIPrintInteractionController *, BOOL, NSError *) =
			^(UIPrintInteractionController *pic, BOOL completed, NSError *error) {
				if (!completed && error)
					NSLog(@"FAILED! due to error in domain %@ with error code %u",
							error.domain, (unsigned int)error.code);
			};
		if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) {
			[pic presentFromBarButtonItem:printButton animated:YES
				completionHandler:completionHandler];
		} else {
			[pic presentAnimated:YES completionHandler:completionHandler];
		}
	}
}

- (void) shareDocument
{
	NSURL *url = [NSURL fileURLWithPath:_filePath];
	UIActivityViewController *cont = [[UIActivityViewController alloc] initWithActivityItems:@[url] applicationActivities:nil];
	cont.popoverPresentationController.barButtonItem = shareButton;
	[self presentViewController:cont animated:YES completion:nil];
	[cont release];
}

- (void) onShare: (id)sender
{
	pdf_document *idoc = pdf_specifics(ctx, doc);
	if (idoc && pdf_has_unsaved_changes(ctx, idoc))
	{
		UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:AlertTitle message:ShareAlertMessage delegate:self cancelButtonTitle:@"Cancel" otherButtonTitles:@"Save and Share", nil];
		[alertView show];
		[alertView release];
	}
	else
	{
		[self shareDocument];
	}
}

- (void) textSelectModeOn
{
	self.navigationItem.rightBarButtonItems = @[tickButton];
	for (UIView<MuPageView> *view in canvas.subviews)
	{
		if (view.number == current)
			[view textSelectModeOn];
	}
}

- (void) textSelectModeOff
{
	for (UIView<MuPageView> *view in canvas.subviews)
	{
		[view textSelectModeOff];
	}
}

- (void) inkModeOn
{
	self.navigationItem.rightBarButtonItems = @[tickButton];
	for (UIView<MuPageView> *view in canvas.subviews)
	{
		if (view.number == current)
			[view inkModeOn];
	}
}

- (void) deleteModeOn
{
	self.navigationItem.rightBarButtonItems = @[deleteButton];
	barmode = BARMODE_DELETE;
}

- (void) inkModeOff
{
	for (UIView<MuPageView> *view in canvas.subviews)
	{
		[view inkModeOff];
	}
}

- (void) onHighlight: (id)sender
{
	barmode = BARMODE_HIGHLIGHT;
	[self textSelectModeOn];
}

- (void) onUnderline: (id)sender
{
	barmode = BARMODE_UNDERLINE;
	[self textSelectModeOn];
}

- (void) onStrikeout: (id)sender
{
	barmode = BARMODE_STRIKE;
	[self textSelectModeOn];
}

- (void) onInk: (id)sender
{
	barmode = BARMODE_INK;
	[self inkModeOn];
}

- (void) onShowSearch: (id)sender
{
	self.navigationItem.rightBarButtonItems =	@[nextButton, prevButton];
	self.navigationItem.leftBarButtonItem = cancelButton;
	self.navigationItem.titleView = searchBar;
	[searchBar becomeFirstResponder];
	barmode = BARMODE_SEARCH;
}

- (void) onTick: (id)sender
{
	for (UIView<MuPageView> *view in canvas.subviews)
	{
		if (view.number == current)
		{
			switch (barmode)
			{
				case BARMODE_HIGHLIGHT:
					[view saveSelectionAsMarkup:PDF_ANNOT_HIGHLIGHT];
					break;

				case BARMODE_UNDERLINE:
					[view saveSelectionAsMarkup:PDF_ANNOT_UNDERLINE];
					break;

				case BARMODE_STRIKE:
					[view saveSelectionAsMarkup:PDF_ANNOT_STRIKE_OUT];
					break;

				case BARMODE_INK:
					[view saveInk];
					break;
					
				case BARMODE_SIGN:
				case BARMODE_Handsign:

					
					break;
			}
		}
	}
	[self showAnnotationMenu];
}

- (void) onDelete: (id)sender
{
	for (UIView<MuPageView> *view in canvas.subviews)
	{
		if (view.number == current)
			[view deleteSelectedAnnotation];
	}
	[self showAnnotationMenu];
}

- (void) onCancel: (id)sender
{
	switch (barmode)
	{
		case BARMODE_SEARCH:
			cancelSearch = YES;
			[searchBar resignFirstResponder];
			[self resetSearch];
			/* fallthrough */
		case BARMODE_ANNOTATION:
		case BARMODE_MORE:
			[self.navigationItem setTitleView: nil];
			[self addMainMenuButtons];
			barmode = BARMODE_MAIN;
			break;
		case BARMODE_HIGHLIGHT:
		case BARMODE_UNDERLINE:
		case BARMODE_STRIKE:
		case BARMODE_DELETE:
			[self showAnnotationMenu];
			[self textSelectModeOff];
			break;
		case BARMODE_INK:
			[self showAnnotationMenu];
			[self inkModeOff];
			break;
		case BARMODE_SIGN:
			[self signModeOff];
			break;
		case BARMODE_Handsign:
			[self showAnnotationMenu];
			[self handsignModeOff];
			break;
	}
}

- (void) onBack: (id)sender
{
	pdf_document *idoc = pdf_specifics(ctx, doc);
	if (idoc && pdf_has_unsaved_changes(ctx, idoc))
	{
		UIAlertView *saveAlert = [[UIAlertView alloc]
			initWithTitle:AlertTitle message:CloseAlertMessage delegate:self
			cancelButtonTitle:@"Discard" otherButtonTitles:@"Save", nil];
		[saveAlert show];
		[saveAlert release];
	}
	else
	{
		[self.navigationController popViewControllerAnimated:YES];
	}
}

- (void) alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
	if ([CloseAlertMessage isEqualToString:alertView.message])
	{
		if (buttonIndex == 1)
			saveDoc(_filePath.UTF8String, doc);

		[alertView dismissWithClickedButtonIndex:buttonIndex animated:YES];
		[self.navigationController popViewControllerAnimated:YES];
	}

	if ([ShareAlertMessage isEqualToString:alertView.message])
	{
		[alertView dismissWithClickedButtonIndex:buttonIndex animated:NO];
		if (buttonIndex == 1)
		{
			saveDoc(_filePath.UTF8String, doc);
			[self shareDocument];
		}
	}
}

- (void) resetSearch
{
	searchPage = -1;
	for (UIView<MuPageView> *view in canvas.subviews)
		[view clearSearchResults];
}

- (void) showSearchResults: (int)count forPage: (int)number
{
	printf("search found match on page %d\n", number);
	searchPage = number;
	[self gotoPage: number animated: NO];
	for (UIView<MuPageView> *view in canvas.subviews)
		if (view.number == number)
			[view showSearchResults: count];
		else
			[view clearSearchResults];
}

- (void) searchInDirection: (int)dir
{
	UITextField *searchField;
	char *needle;
	int start;

	[searchBar resignFirstResponder];

	if (searchPage == current)
		start = current + dir;
	else
		start = current;

	needle = strdup(searchBar.text.UTF8String);

	searchField = nil;
	for (id view in searchBar.subviews)
		if ([view isKindOfClass: [UITextField class]])
			searchField = view;

	[prevButton setEnabled: NO];
	[nextButton setEnabled: NO];
	[searchField setEnabled: NO];

	cancelSearch = NO;

	dispatch_async(queue, ^{
		for (int i = start; i >= 0 && i < fz_count_pages(ctx, doc); i += dir) {
			int n = search_page(doc, i, needle, NULL);
			if (n) {
				dispatch_async(dispatch_get_main_queue(), ^{
					[prevButton setEnabled: YES];
					[nextButton setEnabled: YES];
					[searchField setEnabled: YES];
					[self showSearchResults: n forPage: i];
					free(needle);
				});
				return;
			}
			if (cancelSearch) {
				dispatch_async(dispatch_get_main_queue(), ^{
					[prevButton setEnabled: YES];
					[nextButton setEnabled: YES];
					[searchField setEnabled: YES];
					free(needle);
				});
				return;
			}
		}
		dispatch_async(dispatch_get_main_queue(), ^{
			printf("no search results found\n");
			[prevButton setEnabled: YES];
			[nextButton setEnabled: YES];
			[searchField setEnabled: YES];
			UIAlertView *alert = [[UIAlertView alloc]
				initWithTitle: @"No matches found for:"
				message: @(needle)
				delegate: nil
				cancelButtonTitle: @"Close"
				otherButtonTitles: nil];
			[alert show];
			[alert release];
			free(needle);
		});
	});
}

- (void) onSearchPrev: (id)sender
{
	[self searchInDirection: -1];
}

- (void) onSearchNext: (id)sender
{
	[self searchInDirection: 1];
}

- (void) searchBarSearchButtonClicked: (UISearchBar*)sender
{
	[self onSearchNext: sender];
}

- (void) searchBar: (UISearchBar*)sender textDidChange: (NSString*)searchText
{
	[self resetSearch];
	if (searchBar.text.length > 0) {
		[prevButton setEnabled: YES];
		[nextButton setEnabled: YES];
	} else {
		[prevButton setEnabled: NO];
		[nextButton setEnabled: NO];
	}
}

- (void) onSlide: (id)sender
{
	int number = slider.value;
	if (slider.tracking)
		indicator.text = [NSString stringWithFormat: @" %d of %d ", number+1, fz_count_pages(ctx, doc)];
	else
		[self gotoPage: number animated: NO];
}

- (BOOL) gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
	// For reflow mode, we load UIWebViews into the canvas. Returning YES
	// here prevents them stealing our tap and pinch events.
	return YES;
}

- (void) onTap: (UITapGestureRecognizer*)sender
{
	CGPoint p = [sender locationInView: canvas];
	CGPoint ofs = canvas.contentOffset;
	float x0 = (width - GAP) / 5;
	float x1 = (width - GAP) - x0;
	p.x -= ofs.x;
	p.y -= ofs.y;
	__block BOOL tapHandled = NO;
	for (UIView<MuPageView> *view in canvas.subviews)
	{
		CGPoint pp = [sender locationInView:view];
		if (CGRectContainsPoint(view.bounds, pp))
		{
			MuTapResult *result = [view handleTap:pp];
			__block BOOL hitAnnot = NO;
			[result switchCaseInternal:^(MuTapResultInternalLink *link) {
				[self gotoPage:link.pageNumber animated:NO];
				tapHandled = YES;
			} caseExternal:^(MuTapResultExternalLink *link) {
				// Not currently supported
			} caseRemote:^(MuTapResultRemoteLink *link) {
				// Not currently supported
			} caseWidget:^(MuTapResultWidget *widget) {
				tapHandled = YES;
			} caseAnnotation:^(MuTapResultAnnotation *annot) {
				hitAnnot = YES;
			}];

			switch (barmode)
			{
				case BARMODE_ANNOTATION:
					if (hitAnnot)
						[self deleteModeOn];
					tapHandled = YES;
					break;

				case BARMODE_DELETE:
					if (!hitAnnot)
						[self showAnnotationMenu];
					tapHandled = YES;
					break;

				default:
					if (hitAnnot)
					{
						// Annotation will have been selected, which is wanted
						// only in annotation-editing mode
						[view deselectAnnotation];
					}
					break;
			}

			if (tapHandled)
				break;
		}
	}
	
	if (tapHandled) {
		// Do nothing further
	} else if (p.x < x0) {
		[self gotoPage: current-1 animated: YES];
	} else if (p.x > x1) {
		[self gotoPage: current+1 animated: YES];
	} else {
		if (self.navigationController.navigationBarHidden)
			[self showNavigationBar];
		else if (barmode == BARMODE_MAIN)
			[self hideNavigationBar];
	}
}

- (void) onPinch:(UIPinchGestureRecognizer*)sender
{
	if (sender.state == UIGestureRecognizerStateBegan)
		sender.scale = scale;

	if (sender.scale < MIN_SCALE)
		sender.scale = MIN_SCALE;

	if (sender.scale > MAX_SCALE)
		sender.scale = MAX_SCALE;

	if (sender.state == UIGestureRecognizerStateEnded)
		scale = sender.scale;

	for (UIView<MuPageView> *view in canvas.subviews)
	{
		// Zoom only the visible page until end of gesture
		if (view.number == current || sender.state == UIGestureRecognizerStateEnded)
			[view setScale:sender.scale];
	}
}

- (void) scrollViewWillBeginDragging: (UIScrollView *)scrollView
{
	if (barmode == BARMODE_MAIN)
		[self hideNavigationBar];
}

- (void) scrollViewDidScroll: (UIScrollView*)scrollview
{
	NSLog(@"scrollViewDidScroll");
	// scrollViewDidScroll seems to get called part way through a screen rotation.
	// (This is possibly a UIScrollView bug - see
	// http://stackoverflow.com/questions/4123991/uiscrollview-disable-scrolling-while-rotating-on-iphone-ipad/8141423#8141423 ).
	// This ends up corrupting the current page number, because the calculation
	// 'current = x / width' is using the new value of 'width' before the
	// pages have been resized/repositioned. To avoid this problem, we filter out
	// calls to scrollViewDidScroll during rotation.
	if (_isRotating)
		return;

	if (width == 0)
		return; // not visible yet

	if (scroll_animating)
		return; // don't mess with layout during animations

	float x = canvas.contentOffset.x + width * 0.5f;
	current = x / width;

	[[NSUserDefaults standardUserDefaults] setInteger: current forKey: key];

	indicator.text = [NSString stringWithFormat: @" %d of %d ", current+1, fz_count_pages(ctx, doc)];
	slider.value = current;

	// swap the distant page views out

	NSMutableSet *invisiblePages = [[NSMutableSet alloc] init];
	for (UIView<MuPageView> *view in canvas.subviews) {
		if (view.number != current)
			[view resetZoomAnimated: YES];
		if (view.number < current - 2 || view.number > current + 2)
			[invisiblePages addObject: view];
	}
	for (UIView<MuPageView> *view in invisiblePages)
		[view removeFromSuperview];
	[invisiblePages release]; // don't bother recycling them...

	[self createPageView: current];
	[self createPageView: current - 1];
	[self createPageView: current + 1];

	// reset search results when page has flipped
	if (current != searchPage)
		[self resetSearch];
}

- (void) createPageView: (int)number
{
	if (number < 0 || number >= fz_count_pages(ctx, doc))
		return;
	int found = 0;
	for (UIView<MuPageView> *view in canvas.subviews)
		if (view.number == number)
			found = 1;
	if (!found) {
		UIView<MuPageView> *view;
		if(reflowMode)
			view = [[MuPageViewReflow alloc] initWithFrame:CGRectMake(number * width, 0, width-GAP, height) document:docRef page:number];
		else {
			MuPageViewNormal *viewNormal = [[MuPageViewNormal alloc] initWithFrame:CGRectMake(number * width, 0, width-GAP, height) dialogCreator:self updater:self document:docRef page:number];
			view = viewNormal;
		}
		
		[view setScale:scale];
		[canvas addSubview: view];
		if (showLinks)
			[view showLinks];
		[view release];
	}
}

- (void) gotoPage: (int)number animated: (BOOL)animated
{
	// signmode forbid turning page.
	if(barmode==BARMODE_SIGN || barmode==BARMODE_Handsign)
		return;
	
	if (number < 0)
		number = 0;
	if (number >= fz_count_pages(ctx, doc))
		number = fz_count_pages(ctx, doc) - 1;
	if (current == number)
		return;
	if (animated) {
		// setContentOffset:animated: does not use the normal animation
		// framework. It also doesn't play nice with the tap gesture
		// recognizer. So we do our own page flipping animation here.
		// We must set the scroll_animating flag so that we don't create
		// or remove subviews until after the animation, or they'll
		// swoop in from origo during the animation.

		scroll_animating = YES;
		[UIView beginAnimations: @"MuScroll" context: NULL];
		[UIView setAnimationDuration: 0.4];
		[UIView setAnimationBeginsFromCurrentState: YES];
		[UIView setAnimationDelegate: self];
		[UIView setAnimationDidStopSelector: @selector(onGotoPageFinished)];

		for (UIView<MuPageView> *view in canvas.subviews)
			[view resetZoomAnimated: NO];

		canvas.contentOffset = CGPointMake(number * width, 0);
		slider.value = number;
		indicator.text = [NSString stringWithFormat: @" %d of %d ", number+1, fz_count_pages(ctx, doc)];

		[UIView commitAnimations];
	} else {
		for (UIView<MuPageView> *view in canvas.subviews)
			[view resetZoomAnimated: NO];
		canvas.contentOffset = CGPointMake(number * width, 0);
	}
	current = number;
}

- (void) invokeTextDialog:(NSString *)aString okayAction:(void (^)(NSString *))block
{
	MuTextFieldController *tf = [[MuTextFieldController alloc] initWithText:aString okayAction:block];
	tf.modalPresentationStyle = UIModalPresentationFormSheet;
	[self presentViewController:tf animated:YES completion:nil];
	[tf release];
}

- (void) invokeChoiceDialog:(NSArray *)anArray okayAction:(void (^)(NSArray *))block
{
	MuChoiceFieldController *cf = [[MuChoiceFieldController alloc] initWithChoices:anArray okayAction:block];
	cf.modalPresentationStyle = UIModalPresentationFormSheet;
	[self presentViewController:cf animated:YES completion:nil];
	[cf release];
}

- (void) onGotoPageFinished
{
	scroll_animating = NO;
	[self scrollViewDidScroll: canvas];
}

- (BOOL) shouldAutorotateToInterfaceOrientation: (UIInterfaceOrientation)o
{
	return YES;
}

- (void) willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration
{
	_isRotating = YES;
}

- (void) didRotateFromInterfaceOrientation: (UIInterfaceOrientation)o
{
	_isRotating = NO;

	// We need to set these here, because during the animation we may use a wider
	// size (the maximum of the landscape/portrait widths), to avoid clipping during
	// the rotation.
	canvas.contentSize = CGSizeMake(fz_count_pages(ctx, doc) * width, height);
	canvas.contentOffset = CGPointMake(current * width, 0);
}

- (BOOL) svrSignReady {
#ifdef SVR_SGIN
	if(!_ssCtx) return NO;
	return YES;
#else
	return NO;
#endif
}

- (BOOL) svrLogin {
	if(!_ssCtx) return NO;
	if(_ssCtx->logined) return YES;

	if(!_loginVc)
		_loginVc = [[NTKOViewController alloc]initWithTarget:self viewIndex:0];
	
	[self.navigationController pushViewController:_loginVc animated:YES];
	[_loginVc release];
	_loginVc = nil;
	
	return NO;
}

- (ntko_sign_info*) makeSignInfo_Image {
	ntko_sign_info *info = NULL;
	fz_try(ctx) {
		info = fz_malloc_struct(ctx, ntko_sign_info);
		info->sign_sn = fz_strdup(ctx, [_espFile.sn UTF8String]);
		info->sign_unid = fz_strdup(ctx, info->sign_sn);
		info->sign_name = fz_strdup(ctx, "ImageSign");
#ifdef NTKO_SERVER_SIGN
		info->sign_user = fz_strdup(ctx, _ssCtx->username);
		info->signer = fz_strdup(ctx, _ssCtx->username);
#else
		info->sign_user = NULL;
		info->signer = NULL;
#endif
		info->cert_issure = NULL;
		info->cert_user = NULL;
		info->printmode = 1;
		info->signtype = signtype_image;
	}fz_catch(ctx) {
		NSLog(@"%s", fz_caught_message(ctx));
	}
	return info;
}

- (ntko_sign_info*) makeSignInfo_esp {
	if(barmode!=BARMODE_SIGN || !_espFile) return NULL;
	ntko_sign_info *info = NULL;
	fz_try(ctx) {
		info = fz_malloc_struct(ctx, ntko_sign_info);
		info->sign_sn = fz_strdup(ctx, [_espFile.sn UTF8String]);
		info->sign_unid = fz_strdup(ctx, [_espFile.unid UTF8String]);
		info->sign_name = fz_strdup(ctx, [_espFile.name UTF8String]);
#ifdef NTKO_SERVER_SIGN
		info->sign_user = fz_strdup(ctx, [_espFile.user UTF8String]);
		info->signer = fz_strdup(ctx, _ssCtx->username);
#else
		info->sign_user = NULL;
		info->signer = NULL;
#endif
		info->cert_issure = NULL;
		info->cert_user = NULL;
		info->printmode = 1;
		info->signtype = signtype_sign;
	}fz_catch(ctx) {
		NSLog(@"%s", fz_caught_message(ctx));
	}
	return info;
}

- (ntko_sign_info*) makeSignInfo_Handsign {
	if(barmode!=BARMODE_Handsign) return NULL;
	ntko_sign_info *info = NULL;
	fz_try(ctx) {
		info = fz_malloc_struct(ctx, ntko_sign_info);
		info->sign_sn = fz_strdup(ctx,[[[NSUUID UUID] UUIDString] UTF8String]);
		info->sign_unid = fz_strdup(ctx, info->sign_sn);
		info->sign_name = fz_strdup(ctx, "HandSign");
		//TODO: if no NTKO_SERVER_SIGN set signuser to cert_user,
		// pdf always use certificate to adding signature.
#ifdef NTKO_SERVER_SIGN
		info->sign_user = fz_strdup(ctx, _ssCtx->username);
		info->signer = fz_strdup(ctx, _ssCtx->username);
#else
		info->sign_user = NULL;
		info->signer = NULL;
#endif
		info->cert_issure = NULL;
		info->cert_user = NULL;
		info->printmode = 1;
		info->signtype = signtype_handsign;
	}fz_catch(ctx) {
		NSLog(@"%s", fz_caught_message(ctx));
	}
	return info;
}

#pragma mark - pdf signature
- (void) onSign: (id)sender
{
#ifdef NTKO_SERVER_SIGN
	barmode = BARMODE_SIGN;
	_signstep = SIGN_STEP_SVR_LOGIN;
	BOOL isok = [self svrLogin];
	if(!isok) return;
	[self showSvrEspSel];
#else
	[self signModeOn];
#endif
}

- (void) showSvrEspSel {
	dispatch_async(queue, ^(void) {
		[self startAnimatingInMq];
		NSArray<NTKOEspFile*> *esplist = [[NTKOEspFile svrEsplist]autorelease];
		// [self stopAnimationInMq];
		dispatch_async(dispatch_get_main_queue(), ^(void) {
			[self stopAnimating];
			MuFileselectViewController *vc = [[[MuFileselectViewController alloc]initWithDatasource:esplist Title:nil FileSelDelegate:self FileSelBlock:nil]autorelease];
			vc.backViewController = self;
			[self.navigationController pushViewController:vc animated:YES];
		});
	});
}

- (void) signModeOn {
	barmode = BARMODE_SIGN;
	_signstep = SIGN_STEP_CHOOSE_IMAGEFILE;
	[[self navigationItem] setRightBarButtonItems:[NSArray arrayWithObject:nextstepButton]];
	MuFileselectViewController *vc = [[MuFileselectViewController defaultImageViewController:self SelBlock:nil]autorelease];
	
	NSArray<NTKOEspFile*> *esplist = [[MuFileselectViewController defaultSigns]autorelease];
	[vc addTableSection:esplist Title:@"ntko sign files"];
	
	[self.navigationController pushViewController:vc animated:YES];
}

- (void) onNextstep: (id)sender {
	if(barmode==BARMODE_SIGN && _signstep==SIGN_STEP_GET_SIGN_POSITION) {
		
		UIView<MuPageView> *view = [self curPageView];
		MuSignView *signview = view.signView;
		CGRect rect = signview.rectOfPage;
		id<NTKOTableDs> dsfile = signview.signfile;
		NSData *imgdata = dsfile.imagedata;
		
		if(!CGRectIsEmpty(rect) && imgdata) {
			if(_app)
				z_pdf_drop_sign_appreance(ctx, _app);
			_app = createAppearanceWithNSData(imgdata, rect);
			
			if(_app) [self pushDeviceCreateVc];
			else {
				NSLog(@"create sign appearance faild.");
				[self signModeOff];
			}
		}
	}
	
	if(barmode==BARMODE_Handsign && _signstep==SIGN_STEP_HAND_DRAW_APPEARANCE) {
		UIView<MuPageView> *view = [self curPageView];
		MuHanddrawView *drawView = view.darwView;
		
		z_fpoint_arraylist *al = drawView.strokes;
		CGRect rect = CGRectInset(drawView.strokebounds, -10.0f, -10.0f);
		if(_app) z_pdf_drop_sign_appreance(ctx, _app);
		_app = createAppearanceWithPointArrayList(al, rect);
		z_drop_fpoint_arraylist(ctx, al);
		
		if(_app) [self pushDeviceCreateVc];
		else {
			[self handsignModeOff];
			NSLog(@"create sign appearance faild.");
		}
	}
}

#pragma mark - delegate of MuFileSelectView
// this call back is call by
- (BOOL) fileSelected:(MuFileselectViewController *)vc Selected:(id<NTKOTableDs>)selDs selectdIndexpath:(NSIndexPath *)indexpath
{
	if(barmode==BARMODE_SIGN && ( _signstep==SIGN_STEP_SVR_LOGIN || _signstep==SIGN_STEP_OPEN_ESP || _signstep==SIGN_STEP_CHOOSE_IMAGEFILE) ) {
		if([selDs isKindOfClass:[NTKOEspFile class]]) {
			_signstep = SIGN_STEP_OPEN_ESP;
			NTKOPswCheckViewController *pswVc = [[[NTKOPswCheckViewController alloc]initWithNtkoFile:selDs Delegate:self CheckBlock:nil]autorelease];
			[self.navigationController pushViewController:pswVc animated:YES];
		}
		else if([selDs isKindOfClass:[NTKODsNormalFile class]]) {
			_signstep = SIGN_STEP_GET_SIGN_POSITION;
			UIView<MuPageView>* view = [self curPageView];
			NTKODsNormalFile *dsfile = (NTKODsNormalFile*)selDs;
			[self.navigationController popToViewController:self animated:YES];
			[view imageViewModeOn:dsfile];
			return YES;
		}
	}
	
	if(_signstep==SIGN_STEP_GET_SIGN_DEVICE) {
		if(indexpath.section>0) {
		}
		else {
			if([selDs isKindOfClass:[NTKODsNormalFile class]]) {
				NTKOPswCheckViewController *pswVc = [[[NTKOPswCheckViewController alloc]initWithNtkoFile:selDs Delegate:self CheckBlock:nil]autorelease];
				[self.navigationController pushViewController:pswVc animated:YES];
				return YES;
			}
		}
	}
	return NO;
}

- (void) fileSelCancel:(MuFileselectViewController *)fileselectViewController {
	if(barmode==BARMODE_SIGN &&	_signstep==SIGN_STEP_CHOOSE_IMAGEFILE)
		[self signModeOff];
	if(barmode==BARMODE_SIGN && _signstep==SIGN_STEP_GET_SIGN_DEVICE)
		_signstep = SIGN_STEP_GET_SIGN_POSITION;
}

- (void) fileSelOkay:(MuFileselectViewController *)fileselectViewController selectedfile:(NSString *)file {
}

- (void) pushDeviceCreateVc {
	_signstep = SIGN_STEP_GET_SIGN_DEVICE;
	[MuFileselectViewController pushDefaultPfxSelect:self fileselectedDelegate:self fileselectedblock:nil];
}

- (void) onSvrSignOpened:(NTKOEspFile*)espfile {
	if(BARMODE_SIGN!=barmode || _signstep!=SIGN_STEP_OPEN_ESP)
		return;
	dispatch_async(dispatch_get_main_queue(), ^{
		_signstep = SIGN_STEP_GET_SIGN_POSITION;
		if(_espFile)
			[_espFile release];
		
		_espFile = [espfile retain];
		
		[self.navigationController popToViewController:self animated:YES];
		[self showOkMenu];
		
		UIView<MuPageView>* view = [self curPageView];
		[view imageViewModeOn:espfile];
	});
}

- (ntko_sign_info *) makeSignInfo {
	ntko_sign_info *info = NULL;
	if(barmode==BARMODE_SIGN) info = [self makeSignInfo_esp];
	if(barmode==BARMODE_IMAGESIGN) info = [self makeSignInfo_Image];
	if(barmode==BARMODE_Handsign) info = [self makeSignInfo_Handsign];
	return info;
}

- (void) onSignDeviceCreated {
	if( (barmode!=BARMODE_SIGN && barmode!=BARMODE_Handsign && barmode!=BARMODE_IMAGESIGN) ||
	   _signstep!=SIGN_STEP_GET_SIGN_DEVICE || !_device || !_app)
		return;
	
	UIView<MuPageView> *view = [self curPageView];
	if(view) {
		_signstep = SIGN_SETP_ADD_SIGN;
		ntko_sign_info *info = [self makeSignInfo];
		[view addsign: _app signdevice:_device SignInfo:info openedFile:_filePath];
	}
	
	dispatch_async(dispatch_get_main_queue(), ^{
		[self.navigationController popToViewController:self animated:YES];
		[self signModeOff];
		[self handsignModeOff];
		[self showAnnotationMenu];
	});
}


#pragma mark - delegate of password check viewcontroller
- (BOOL) onPswCheck:(NTKOPswCheckViewController*)checkVc Ds:(id<NTKOTableDs>)ds Password:(NSString *)psw {
	BOOL closePswVc = NO;
	
	// _signstep==SIGN_STEP_OPEN_ESP
	if([ds isKindOfClass:[NTKOEspFile class]]) {
		NTKOEspFile *signfile = (NTKOEspFile*)ds;
		closePswVc = [signfile open:psw];
		if(closePswVc) {
			[self onSvrSignOpened:signfile];
		}
	}
	
	// _signstep == SIGN_STEP_GET_SIGN_DEVICE
	if( (barmode==BARMODE_SIGN || barmode==BARMODE_Handsign) && _signstep==SIGN_STEP_GET_SIGN_DEVICE) {
		if([ds isKindOfClass:[NTKOPfxFile class]]) {
			NTKOPfxFile *pfxfile = (NTKOPfxFile*)ds;
			if(_device) z_drop_device(ctx, _device);
			_device = NULL;
			_device = [pfxfile createOpsslDev:psw];
			if(_device) {
				[self onSignDeviceCreated];
			}
		}
	}

	return closePswVc;
}

- (void) onPswViewClose:(int)remainChances {
	
	if(barmode==BARMODE_SIGN && _signstep==SIGN_STEP_OPEN_ESP) {
	}
}


//- (BOOL) deviceCreateOk:(z_device *)device
//{
//	if(_signstep==SIGN_STEP_GET_SIGN_DEVICE) {
//		_signstep = SIGN_SETP_ADD_SIGN;
//		UIView<MuPageView> *view = [self curPageView];
//		
//		if(view) {
//			if(_device) z_drop_device(ctx, _device);
//			_device = z_keep_device(ctx, device);
//			
//			if(_device && _app)
//				[view addsign:_app signdevice:_device];
//			else
//				NSLog(@"invalid sign params");
//		}
//		[self signModeOff];
//		[self handsignModeOff];
//		[self showAnnotationMenu];
//
//	}
//	return NO;
//}

- (void) deviceCreateFailed {
	if(barmode==BARMODE_Handsign)
		[self handsignModeOff];
	if(barmode==BARMODE_SIGN)
		[self signModeOff];
	
	[self showAnnotationMenu];
}

- (void) signModeOff {
	_signstep = SIGN_STEP_NOT_SIGINING;
	[self.navigationController popToViewController:self animated:YES];
	[self showAnnotationMenu];
	
	UIView<MuPageView> *view = [self curPageView];
	[view imageViewModeOff];
	
	if(_app)
		z_pdf_drop_sign_appreance(ctx, _app);
	_app = NULL;
	if(_device)
		z_drop_device(ctx, _device);
	_device = NULL;
	
	[_espFile release];
	_espFile = nil;
}

- (void) onHandsign: (id)sender {
#ifdef NTKO_SERVER_SIGN
	barmode = BARMODE_Handsign;
	_signstep = SIGN_STEP_SVR_LOGIN;
	BOOL isok = [self svrLogin];
	if(!isok) return;
#endif
	[self handsignModeOn];
}

- (void) handsignModeOn {
	barmode = BARMODE_Handsign;
	_signstep = SIGN_STEP_HAND_DRAW_APPEARANCE;
	
	[[self navigationItem] setRightBarButtonItems:[NSArray arrayWithObject:nextstepButton]];
	
	UIView<MuPageView> *view = [self curPageView];
	if(!view) return;
	[view handsignModeOn];
}

- (void) handsignModeOff {
	_signstep = SIGN_STEP_NOT_SIGINING;
	UIView<MuPageView> *view = [self curPageView];
	if(!view) return;
	[view handsignModeOff];
	
	if(_app)
		z_pdf_drop_sign_appreance(ctx, _app);
	_app = NULL;
	if(_device)
		z_drop_device(ctx, _device);
	_device = NULL;
}

- (UIView<MuPageView>*) curPageView {
	for (UIView<MuPageView> *v in [canvas subviews]) {
		if ([v number] == current) {
			return v;
		}
	}
	return nil;
}

- (void) startAnimatingInMq {
	dispatch_async(dispatch_get_main_queue(), ^{
		[self startAnimating];
	});
}
- (void) startAnimating {
	if(!_holdingView)
		[[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhiteLarge];
	[_holdingView startAnimating];
	[self.view addSubview: _holdingView];
}

-(void)stopAnimationInMq {
	dispatch_async(dispatch_get_main_queue(), ^{
		[self stopAnimating];
	});
}

- (void) stopAnimating {
	if(_holdingView) {
		[_holdingView stopAnimating];
		[_holdingView removeFromSuperview];
	}
}

#pragma mark ----Login Delegate------
- (void) OnLoginOk {
	if(barmode==BARMODE_SIGN) {
		dispatch_async(queue, ^(void)
		{
			NSArray<NTKOEspFile*> *esplist = [[NTKOEspFile svrEsplist]autorelease];
			MuFileselectViewController *selVc = [[MuFileselectViewController alloc]
				initWithDatasource:esplist Title:nil FileSelDelegate:self FileSelBlock:nil];
			
			if(_ssCtx->rights.permit_imagesign) {
				NSArray<NTKODsNormalFile*> *imgfiles = [[MuFileselectViewController defaultImages]autorelease];
				[selVc addTableSection:imgfiles Title:@"_imagefiles"];
			}
			
			dispatch_async(dispatch_get_main_queue(), ^(void) {
				[self.navigationController pushViewController:selVc animated:YES];
				[selVc release];
			});
			
		});
	}
	
	if(barmode==BARMODE_Handsign) {
		dispatch_async(dispatch_get_main_queue(), ^(void) {
			[self.navigationController popToViewController:self animated:YES];
			if(_ssCtx->rights.permit_handsign)
				[self handsignModeOn];
			else {
				[self handsignModeOff];
				NSLog(@"hand sign not permited");
			}
		});
	}
}
@end
