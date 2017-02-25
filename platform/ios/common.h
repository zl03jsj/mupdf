#ifndef MuPDF_common_h
#define MuPDF_common_h

#include <UIKit/UIKit.h>

#undef ABS
#undef MIN
#undef MAX

#include "mupdf/fitz.h"
#include "mupdf/z/z_pdf.h"
#include "mupdf/z/ntko_svr.h"

#define SVR_SIGN


extern fz_context *ctx;
extern dispatch_queue_t queue;
extern float screenScale;

CGSize fitPageToScreen(CGSize page, CGSize screen);
int search_page(fz_document *doc, int number, char *needle, fz_cookie *cookie);
fz_rect search_result_bbox(fz_document *doc, int i);
CGDataProviderRef CreateWrappedPixmap(fz_pixmap *pix);
CGImageRef CreateCGImageWithPixmap(fz_pixmap *pix, CGDataProviderRef cgdata);

float z_cg_insertPoint(fz_context *ctx, z_fpoint_array *arr, CGPoint lastpoint, UInt64 lastms,
					   float lastwidth, CGPoint point, UInt64 ms);
void z_cg_insertLastPoint(fz_context *ctx, z_fpoint_array *arr, CGPoint e);
float z_cg_distance(CGPoint first, CGPoint second);

fz_point z_CGPoint2Point(CGPoint point);
CGPoint z_Point2CGPoint(fz_point point);
CGRect z_CGRectExpandToPoint(CGRect rect, CGPoint p);

typedef struct ntko_svrsign_context_s {
	bool logined;
	ntko_server_info svrinfo;
	ntko_user_rights rights;
	ntko_http_response_status status;
} ntko_svrsign_context;

extern ntko_svrsign_context *_ssCtx;

bool z_init_ssCtx();
bool z_free_ssCtx();

#endif





















