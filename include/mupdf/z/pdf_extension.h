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
fz_buffer *get_file_data(fz_context *ctx, char *filename, int ofs, int len);
ImageType img_recognize(char *filename);
pdf_document *pdf_open_document_with_filename(fz_context *ctx, const char *file, char *password);
pdf_document *pdf_open_document_with_filestream(fz_context * ctx, fz_stream *file, char * password);
fz_buffer *deflate_buffer_fromdata(fz_context *ctx,char *p, int n);
fz_buffer *fz_pixmap_rgb(fz_context *ctx, fz_pixmap *pixmap);
fz_buffer *fz_pixmap_rgb_mask(fz_context *ctx, fz_pixmap *pixmap);
pdf_obj *add_image_xobj(fz_context *ctx, pdf_document *doc, Xobj_Image *xi);
pdf_obj *pdf_add_extstate(fz_context *ctx, pdf_document *doc);
pdf_obj *pdf_add_pixmap(fz_context *ctx, pdf_document *doc, fz_pixmap *pixmap);
pdf_obj *pdf_add_content(fz_context *ctx, pdf_document *doc, char* xobjname, int x, int y, int w, int h);
int pdf_page_add_content(fz_context *ctx, pdf_document *doc, pdf_obj *page, pdf_obj *objref);
int pdf_resource_add_xobj(fz_context *ctx, pdf_document *doc, pdf_obj *resobj, char *name, pdf_obj *ref);
int pdf_resource_add_extgstate(fz_context *ctx, pdf_document *doc, pdf_obj *resobj, const char *name, pdf_obj *ref);
int pdf_add_image_with_filestream(fz_context *ctx, fz_stream*file, fz_buffer*imgbf, char *outfile, int pageno, int x, int y, int w, int h, char *password);
int pdf_add_image_with_document(fz_context *ctx, pdf_document *doc, fz_buffer*imgbf, int pageno, int x, int y, int w, int h);
int pdf_add_content_stream(fz_context *ctx, pdf_document *doc, pdf_obj *page, fz_buffer *buffer);
void stderr_tofile(char *filename);
void stderr_restore();

void z_pdf_incremental_save_document(fz_context *ctx, pdf_document *doc, const char *savefile, const char *orignalfile);

#ifdef __cplusplus
}
#endif
#endif
