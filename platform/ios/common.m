#include "common.h"
#include "mupdf/_z/z_algorithm.h"

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

	fz_stext_sheet *sheet = fz_new_stext_sheet(ctx);
	fz_stext_page *text = fz_new_stext_page(ctx);
	fz_device *dev = fz_new_stext_device(ctx, sheet, text);
	fz_run_page(ctx, page, dev, &fz_identity, cookie);
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
	CGImageRef cgimage = CGImageCreate(w, h, 8, 32, 4 * w, cgcolor, kCGBitmapByteOrderDefault, cgdata, NULL, NO, kCGRenderingIntentDefault);
	CGColorSpaceRelease(cgcolor);
	return cgimage;
}

z_point_width CreateZPointWidth(float x, float y, float w) {
	z_point_width p = {{x,y}, w};
	return p;
}

float InsertPoint(NSMutableArray *arr, CGPoint lastpoint, UInt64 lastms,
	float lastwidth, CGPoint point, UInt64 ms) {
	if(!arr) return 0;
	long count = [arr count];
	z_point zp = {point.x, point.y};
	if( 0==count ){
		z_point_width p = {zp, 0.1};
		[arr addObject:[NSValue valueWithBytes:&p objCType:@encode(z_point_width)]];
		return 0.1;
	}
	
	float step = count > 4 ? 0.1: 0.2;
	z_point_time bt = { {lastpoint.x,lastpoint.y}, lastms};
	z_point_time et = { zp, ms};
	float w = z_get_width(bt, et, lastwidth, step);
	if( 1==count ) {
		z_point_width p = { {(bt.p.x + et.p.x + 1) / 2, (bt.p.y + et.p.y +1) / 2},
			(w + lastwidth) / 2};
		[arr addObject:[NSValue valueWithBytes:&p objCType:@encode(z_point_width)]];
		return w;
	}
	
	z_point_width bw = { {lastpoint.x,lastpoint.y},lastwidth};
	z_point_width ew = { zp, w};
	z_point_width cw;
	[[arr lastObject] getValue:&cw];
	
	
	return w;
}







