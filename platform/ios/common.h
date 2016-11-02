#ifndef MuPDF_common_h
#define MuPDF_common_h

#include <UIKit/UIKit.h>

#undef ABS
#undef MIN
#undef MAX

#include "mupdf/fitz.h"
#include "mupdf/z_/z_pdf.h"

extern fz_context *ctx;
extern dispatch_queue_t queue;
extern float screenScale;

CGSize fitPageToScreen(CGSize page, CGSize screen);

int search_page(fz_document *doc, int number, char *needle, fz_cookie *cookie);

fz_rect search_result_bbox(fz_document *doc, int i);

CGDataProviderRef CreateWrappedPixmap(fz_pixmap *pix);

CGImageRef CreateCGImageWithPixmap(fz_pixmap *pix, CGDataProviderRef cgdata);

float z_IOS_insertPoint(NSMutableArray *arr, CGPoint lastpoint, UInt64 lastms,
	float lastwidth, CGPoint point, UInt64 ms);
void z_IOS_insertLastPoint(NSMutableArray *arr, CGPoint e);
CGPoint z_get_stored_CGPoint(NSArray *arr, int index);
float   z_get_stored_Width  (NSArray *arr, int index);
z_point_width z_stored_point(NSArray *arr, int index);
CGRect CGRectExpendTo(CGRect r, CGPoint p);


CGFloat distanceBetweenPoints(CGPoint first, CGPoint second);
CGFloat angleBetweenPoints(CGPoint first, CGPoint second);
CGFloat angleBetweenLines(CGPoint line1Start, CGPoint line1End, CGPoint line2Start, CGPoint line2End);


// extends by zl [2016/10/14 5:34]
// for add pdf signature
typedef struct z_device_s z_device;

typedef int (*dosign_func)(z_device *);

typedef struct z_pdf_sign_param_s {
	fz_context *ctx;
	pdf_document *doc;
	pdf_page *page;
	fz_image *image;
	pdf_annot *annot;
	pdf_obj *byterange;
	fz_rect rect;
	char *orignalfilename;
	char *savefilename;
} z_pdf_sign_param;

struct z_device_s {
	z_pdf_sign_param *param;
	dosign_func pdf_dosign_func;
};

typedef struct z_openssl_device_s {
	z_device *device;
	char *sigfile;
	char *password;
} z_openssl_device;

int z_dosgin(z_device *device)
{
	return device->pdf_dosign_func(device);
}

int z_openssl_dosign(z_device *device)
{
	z_openssl_device *dev = (z_openssl_device*)device;
	z_pdf_sign_param *param = device->param;
	fz_context *ctx = param->ctx;
	pdf_document *doc = param->doc;
	int pageno = pdf_lookup_page_number(ctx, param->doc, param->page->me);
	
	if( !param->annot ) {
		pdf_annot *annot = pdf_create_annot(ctx, param->doc, param->page, FZ_ANNOT_WIDGET);
		pdf_obj *annotobj = annot->obj;
		pdf_dict_puts_drop(ctx, annotobj, "Rect",
						   pdf_new_rect(ctx, param->doc, &param->rect));
		
	}

	z_pdf_sign_signature_with_image();
	
}

int z_ekey_dosign(z_ekey_sign_device *device)
{
	
}





void z_showSigndataDialog(UIViewController *viewcontroller,z_pdf_sign_param *param);

#endif





















