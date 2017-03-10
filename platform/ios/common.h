#ifndef MuPDF_common_h
#define MuPDF_common_h

#include <UIKit/UIKit.h>

#undef ABS
#undef MIN
#undef MAX

#include "mupdf/fitz.h"
#include "mupdf/z/z_pdf.h"

#ifdef __cplusplus
extern "C" {
#endif
	
// #define SVR_SIGN
#include "mupdf/z/ntko_svr.h"

extern fz_context *ctx;
extern dispatch_queue_t queue;
extern float screenScale;

CGSize fitPageToScreen(CGSize page, CGSize screen);
int search_page(fz_document *doc, int number, char *needle, fz_cookie *cookie);
fz_rect search_result_bbox(fz_document *doc, int i);
CGDataProviderRef CreateWrappedPixmap(fz_pixmap *pix);
CGImageRef CreateCGImageWithPixmap(fz_pixmap *pix, CGDataProviderRef cgdata);
UIImage *newImageWithPixmap(fz_pixmap *pix, CGDataProviderRef cgdata, float scale);

float z_cg_insertPoint(fz_context *ctx, z_fpoint_array *arr, CGPoint lastpoint, UInt64 lastms,
					   float lastwidth, CGPoint point, UInt64 ms);
void z_cg_insertLastPoint(fz_context *ctx, z_fpoint_array *arr, CGPoint e);
float z_cg_distance(CGPoint first, CGPoint second);

fz_point z_CGPoint2Point(CGPoint point);
CGPoint z_Point2CGPoint(fz_point point);
CGRect z_CGRectExpandToPoint(CGRect rect, CGPoint p);

typedef struct ntko_svrsign_context_s {
	bool logined;
	char *username;
	char *password;
	ntko_server_info svrinfo;
	ntko_user_rights rights;
	ntko_http_response_status status;
	ntko_sign_options options;
} ntko_svrsign_context;

extern ntko_svrsign_context *_ssCtx;
	
bool z_init_ssCtx();
bool z_free_ssCtx();

NSString *getLoginuser();
	
bool download_server_esp(ntko_server_espinfo *espinfo);
	
bool svr_sign_log(ntko_sign_info *signinfo);

BOOL isFileImage(NSString *file);

BOOL isFilePfx(NSString *file);

BOOL fileIsInPath(NSArray *paths, NSString *file);

BOOL fileHasSuffixs(NSArray *suffixs, NSString *file);
	

	
#ifdef __cplusplus
}
#endif

#endif





















