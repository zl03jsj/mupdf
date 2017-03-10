#include "common.h"

fz_context *ctx;
dispatch_queue_t queue;
float screenScale = 1;

CGSize fitPageToScreen(CGSize page, CGSize screen)
{
	float hscale = screen.width / page.width;
	float vscale = screen.height / page.height;
	float scale = fz_min(hscale, vscale);
	hscale = floorf(page.width * scale) / page.width;
	vscale = floorf(page.height * scale) / page.height;
	return CGSizeMake(hscale, vscale);
}

static int hit_count = 0;
static fz_rect hit_bbox[500];

int search_page(fz_document *doc, int number, char *needle, fz_cookie *cookie)
{
	fz_page *page = fz_load_page(ctx, doc, number);
	fz_rect mediabox;
	fz_stext_sheet *sheet = fz_new_stext_sheet(ctx);
	fz_stext_page *text = fz_new_stext_page(ctx, fz_bound_page(ctx, page, &mediabox));
	fz_device *dev = fz_new_stext_device(ctx, sheet, text, NULL);
	fz_run_page(ctx, page, dev, &fz_identity, cookie);
	fz_close_device(ctx, dev);
	fz_drop_device(ctx, dev);

	hit_count = fz_search_stext_page(ctx, text, needle, hit_bbox, nelem(hit_bbox));

	fz_drop_stext_page(ctx, text);
	fz_drop_stext_sheet(ctx, sheet);
	fz_drop_page(ctx, page);
	
	return hit_count;
}

fz_rect search_result_bbox(fz_document *doc, int i)
{
	return hit_bbox[i];
}

static void releasePixmap(void *info, const void *data, size_t size)
{
	if (queue)
		dispatch_async(queue, ^{
			fz_drop_pixmap(ctx, info);
		});
	else
	{
		fz_drop_pixmap(ctx, info);
	}
}

CGDataProviderRef CreateWrappedPixmap(fz_pixmap *pix)
{
	unsigned char *samples = fz_pixmap_samples(ctx, pix);
	int w = fz_pixmap_width(ctx, pix);
	int h = fz_pixmap_height(ctx, pix);
	return CGDataProviderCreateWithData(pix, samples, w * 4 * h, releasePixmap);
}

CGImageRef CreateCGImageWithPixmap(fz_pixmap *pix, CGDataProviderRef cgdata)
{
	int w = fz_pixmap_width(ctx, pix);
	int h = fz_pixmap_height(ctx, pix);
	CGColorSpaceRef cgcolor = CGColorSpaceCreateDeviceRGB();
	CGImageRef cgimage = CGImageCreate(w, h, 8, 32, 4 * w, cgcolor, kCGBitmapByteOrderDefault,
									   cgdata, NULL, NO, kCGRenderingIntentDefault);
	CGColorSpaceRelease(cgcolor);
	
	return cgimage;
}

UIImage *newImageWithPixmap(fz_pixmap *pix, CGDataProviderRef cgdata, float scale)
{
	CGImageRef cgimage = CreateCGImageWithPixmap(pix, cgdata);
	UIImage *image = [[UIImage alloc] initWithCGImage: cgimage scale: scale orientation: UIImageOrientationUp];
	CGImageRelease(cgimage);
	return image;
}

fz_point z_CGPoint2Point(CGPoint point) {
	fz_point p = {point.x, point.y};
	return p;
}

CGPoint z_Point2CGPoint(fz_point point) {
	return CGPointMake(point.x, point.y);
}

float z_cg_insertPoint(fz_context *ctx, z_fpoint_array *arr, CGPoint lastpoint, UInt64 lastms,
					   float lastwidth, CGPoint point, UInt64 ms)
{
	return z_insertPoint(ctx, arr, z_CGPoint2Point(lastpoint), lastms, lastwidth,
						 z_CGPoint2Point(point), ms);
}

void z_cg_insertLastPoint(fz_context *ctx, z_fpoint_array *arr, CGPoint e)
{
	z_insertLastPoint(ctx, arr, z_CGPoint2Point(e));
}

float z_cg_distance (CGPoint first, CGPoint second) {
	CGFloat deltaX = second.x - first.x;
	CGFloat deltaY = second.y - first.y;
	return sqrt(deltaX*deltaX + deltaY*deltaY);
};

CGRect z_CGRectExpandToPoint(CGRect rect, CGPoint p) {
	if(rect.origin.x > p.x) {
		rect.size.width = rect.size.width + (rect.origin.x - p.x);
		rect.origin.x = p.x;
	}
	else if( (rect.origin.x + rect.size.width) < p.x ) {
		rect.size.width = p.x - rect.origin.x;
	}
	
	if(rect.origin.y > p.y) {
		rect.size.height = rect.size.height + (rect.origin.y - p.y);
		rect.origin.y = p.y;
	}
	else if( (rect.origin.y + rect.size.height) < p.y ) {
		rect.size.height = p.y - rect.origin.y;
	}
	return rect;
}

#define pi 3.14159265358979323846
#define degreesToRadian(x)  (pi * x / 180.0)
#define radiansToDegrees(x) (180.0 * x / pi)
CGFloat distanceBetweenPoints (CGPoint first, CGPoint second) {
	CGFloat deltaX = second.x - first.x;
	CGFloat deltaY = second.y - first.y;
	return sqrt(deltaX*deltaX + deltaY*deltaY );
};

CGFloat angleBetweenPoints(CGPoint first, CGPoint second) {
	CGFloat height = second.y - first.y;
	CGFloat width = first.x - second.x;
	CGFloat rads = atan(height/width);
	return radiansToDegrees(rads);
}

CGFloat angleBetweenLines(CGPoint line1Start, CGPoint line1End, CGPoint line2Start, CGPoint line2End) {
	CGFloat a = line1End.x - line1Start.x;
	CGFloat b = line1End.y - line1Start.y;
	CGFloat c = line2End.x - line2Start.x;
	CGFloat d = line2End.y - line2Start.y;
	CGFloat rads = acos(((a*c) + (b*d)) / ((sqrt(a*a + b*b)) * (sqrt(c*c + d*d))));
	return radiansToDegrees(rads);
}

ntko_svrsign_context *_ssCtx = null;

bool z_init_ssCtx() {
	if(!ctx) return false;
	fz_try(ctx)
		_ssCtx = fz_malloc(ctx, sizeof(ntko_svrsign_context));
	fz_catch(ctx) {
		fz_warn(ctx, "%s", fz_caught_message(ctx));
		return false;
	}
	return true;
}

bool z_free_ssCtx() {
	if(!ctx) return false;
	fz_try(ctx) {
		if(_ssCtx->username) fz_free(ctx, _ssCtx->username);
		if(_ssCtx->password) fz_free(ctx, _ssCtx->password);
		
		if(_ssCtx->status.data) fz_drop_buffer(ctx, _ssCtx->status.data);
		
		if(_ssCtx->svrinfo.lic_username) fz_free(ctx, _ssCtx->svrinfo.lic_username);
		if(_ssCtx->svrinfo.rooturl) fz_free(ctx, _ssCtx->svrinfo.rooturl);
		if(_ssCtx->svrinfo.servername) fz_free(ctx, _ssCtx->svrinfo.servername);
		if(_ssCtx->svrinfo.settingurl) fz_free(ctx, _ssCtx->svrinfo.settingurl);
		if(_ssCtx->svrinfo.version) fz_free(ctx, _ssCtx->svrinfo.version);
		
		fz_free(ctx, _ssCtx);
		_ssCtx = NULL;
	}
	fz_catch(ctx) {
		fz_warn(ctx, "%s", fz_caught_message(ctx));
		return false;
	}
	return true;
}

// static NSString* imagefilesuffixs[] = {@".bmp", @".jpeg",@".jpg",@".bmp",@".png", @".gif"};

BOOL isFileImage(NSString *file)
{
	// NSString const* image_suffix[] = {@".bmp",@".jpeg",@".jpg",@".bmp",@".png", @".gif"};
	NSArray *image_suffixs = @[@".bmp",@".jpeg",@".jpg",@".bmp",@".png", @".gif"];
	return fileHasSuffixs(image_suffixs, file);
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

#ifdef NTKO_SERVER_SIGN
#pragma message("NTKO_SERVER_SIGN is defined!!!!")
NSString * getLoginuser() {
	if(_ssCtx->logined && _ssCtx->username) {
		return [NSString stringWithUTF8String:_ssCtx->username];
	}
	else return nil;
}

bool download_server_esp(ntko_server_espinfo *espinfo) {
	if(!_ssCtx || !_ssCtx->logined)
		return false;
	return ntko_http_download_esp(ctx, &_ssCtx->svrinfo, espinfo, &_ssCtx->status);
}

#else

NSString * getLoginuser() {
	NSLog(@"not defined NTKO_SERVER_SIGN macro");
	return nil;
}

bool download_server_esp(ntko_server_espinfo *espinfo) {
	NSLog(@"not defined NTKO_SERVER_SIGN macro");
	return false;
}

BOOL ntko_dologin() {
	NSLog(@"not defined NTKO_SERVER_SIGN macro");
	return NO;
}

#endif




























