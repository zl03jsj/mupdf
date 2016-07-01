#ifndef MuPDF_common_h
#define MuPDF_common_h

#include <UIKit/UIKit.h>

#undef ABS
#undef MIN
#undef MAX

#include "mupdf/fitz.h"

extern fz_context *ctx;
extern dispatch_queue_t queue;
extern float screenScale;

CGSize fitPageToScreen(CGSize page, CGSize screen);

int search_page(fz_document *doc, int number, char *needle, fz_cookie *cookie);

fz_rect search_result_bbox(fz_document *doc, int i);

CGDataProviderRef CreateWrappedPixmap(fz_pixmap *pix);

CGImageRef CreateCGImageWithPixmap(fz_pixmap *pix, CGDataProviderRef cgdata);

float z_insertPoint(NSMutableArray *arr, CGPoint lastpoint, UInt64 lastms,
	float lastwidth, CGPoint point, UInt64 ms);
void z_insertLastPoint(NSMutableArray *arr, CGPoint e);
CGPoint z_get_stored_CGPoint(NSArray *arr, int index);
float   z_get_stored_Width  (NSArray *arr, int index);

CGRect CGRectExpendTo(CGRect r, CGPoint p);
#endif
