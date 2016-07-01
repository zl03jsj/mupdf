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

z_point_width z_point_width_new(float x, float y, float w) {
	z_point_width p = {{x,y}, w};
	return p;
}

static float z_lineWidth(z_point_time bt, z_point_time et, float bwidth,float step){
	const float max_speed = 3.0f;
	// const float min_speed = 0.2f;
	float d = z_distance(bt.p, et.p);
	float s = d / (et.t - bt.t); s = s > max_speed ? max_speed : s;
	float w = (max_speed-s) / max_speed;
	float max_dif = d * step;
	if( w<0.05 ) w = 0.05;
	if( fabs( w-bwidth ) > max_dif ){
		if( w > bwidth )
			w = bwidth + max_dif;
		else
			w = bwidth - max_dif;
	}
	// printf("d:%.4f, time_diff:%lld, speed:%.4f, width:%.4f\n", d, et.t-bt.t, s, w);
	return w;
}

float z_insertPoint(NSMutableArray *arr, CGPoint lastpoint, UInt64 lastms,
	float lastwidth, CGPoint point, UInt64 ms) {
	if(!arr) return 0;
	long count = [arr count];
	z_point zp = {point.x, point.y};
	if( 0==count ){
		z_point_width p = {zp, 0.1};
		[arr addObject:[NSValue valueWithBytes:&p objCType:@encode(z_point_width)]];
		return 0.1;
	}
	float step = count > 4 ? 0.01: 0.02;
	z_point_time bt = { {lastpoint.x,lastpoint.y}, lastms};
	z_point_time et = { zp, ms};
	float w = z_lineWidth(bt, et, lastwidth, step);
	z_points *points = z_points_new(51);
	if( 1==count ) {
		z_point_width p = { {(bt.p.x + et.p.x + 1) / 2, (bt.p.y + et.p.y +1) / 2},
			(w + lastwidth) / 2};
		z_points_add_differentation(points, p);
	}
	else {
		z_point_width bw;
		[[arr lastObject] getValue:&bw];
		z_point c =  {lastpoint.x,lastpoint.y};
		z_point_width ew = {{(lastpoint.x + point.x)/2, (lastpoint.y + point.y)/2},
			(lastwidth + w) / 2};
		z_square_bezier(points, bw, c, ew);
	}
	
	for(int i=0; i<points->count; i++) {
		[arr addObject:[NSValue valueWithBytes:(points->data+i) objCType:@encode(z_point_width)]];
	}
	z_points_release(points);
	return w;
}

void z_insertLastPoint(NSMutableArray *arr, CGPoint e) {
	if(!arr) return;
	long count = [arr count];
	if( count==0 ) return;
	z_points *points = z_points_new(51);
	z_point_width p = { {e.x, e.y}, 0.1};
	z_points_add_differentation(points, p);
	for(int i=0; i<points->count; i++) {
		[arr addObject:[NSValue valueWithBytes:(points->data+i) objCType:@encode(z_point_width)]];
	}
	z_points_release(points);
}

z_point_width z_stored_point(NSArray *arr, int index) {
	z_point_width pt;
	[[arr objectAtIndex:index] getValue:&pt];
	return pt;
}

CGPoint z_get_stored_CGPoint(NSArray *arr, int index) {
	z_point_width pt = z_stored_point(arr, index);
	CGPoint p = {pt.p.x, pt.p.y};
	return p;
}

float z_get_stored_Width(NSArray *arr, int index){
	z_point_width pt = z_stored_point(arr, index);
	return pt.w;
}


CGRect CGRectExpendTo(CGRect r, CGPoint p){
	if(CGRectContainsPoint(r, p))
		return r;
	CGPoint o = r.origin;
	CGSize  s = r.size;
	if( o.x > p.x) o.x = p.x;
	else{
		float x_dif = p.x - o.x;
		if( s.width <x_dif )
			s.width = x_dif;
	}
	if( o.y > p.y) o.y = p.y;
	else {
		float y_dif = p.y - r.origin.y;
		if( s.height <y_dif )
			s.height = y_dif;
	}
	return CGRectMake(o.x, o.y, s.width, s.height);
}



