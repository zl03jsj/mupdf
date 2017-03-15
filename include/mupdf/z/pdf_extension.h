#ifndef MUPDF_EXTENSIONS_H
#define MUPDF_EXTENSIONS_H

#include "mupdf/pdf.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	image_type_unkown = -1,
	image_type_jpg,
	image_type_gif,
	image_type_png,
	image_type_bmp,
} ImageType;

typedef struct {
	int w, h, n;
	char *filter;
	char *colorspace;
	fz_buffer *data;
	pdf_obj *maskobj;
}Xobj_Image;

#define extension_okay 0

char *new_time_string(fz_context *ctx);

char *new_unique_string(fz_context *ctx, char *prefix, char *suffix);
ImageType img_recognize(char *filename);

pdf_document *pdf_open_document_with_filename(fz_context *ctx, const char *file, char *password);

pdf_document *pdf_open_document_with_filestream(fz_context * ctx, fz_stream *file, char * password);

fz_buffer *deflate_buffer_fromdata(fz_context *ctx,char *p, int n);

int pdf_add_image_with_filestream(fz_context *ctx, fz_stream*file, fz_buffer*imgbf,
	int pageno, int x, int y, int w, int h, char *savefile);

int pdf_add_image_with_document(fz_context *ctx, pdf_document *doc, fz_buffer*imgbf, int pageno, int x, int y, int w, int h);

int pdf_add_image_with_filename(fz_context *ctx, char *pdffile, char *imgfile, int pageno, int x, int y, int w, int h, char *savefile);

int pdf_add_imagefile(fz_context *ctx, pdf_document *doc, char *imgfile, int pageno, int x, int y, int w, int h);

// #define redirect_error_output
#ifdef redirect_error_output
void stderr_tofile(char *filename);
void stderr_restore();
#endif

void z_pdf_incremental_save_document(fz_context *ctx, pdf_document *doc, const char *savefile, const char *orignalfile);
#ifdef __cplusplus
}
#endif
#endif
