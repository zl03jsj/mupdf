/*
 * =====================================================================================
 *
 *       Filename:  z_sign.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/17/2016 17:35:56
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef Z_SIGN_H
#define Z_SIGN_H

#include "mupdf/pdf.h"

typedef struct z_device_s z_device;
typedef struct z_pdf_sign_appearance_s z_pdf_sign_appearance;

typedef fz_buffer* (*z_sign_get_digest_fn)(fz_context *ctx, pdf_document *doc, z_device *device, char *filename, pdf_obj *byte_range);

typedef void (*z_sign_signature_fn)(fz_context *, z_device *, pdf_document*, pdf_annot *annot, z_pdf_sign_appearance *app);

typedef void (*z_sign_set_appearance_fn)(fz_context *, pdf_document *, pdf_annot *, z_pdf_sign_appearance *);
typedef void*(*z_sign_keep_appearance_fn)(fz_context*, z_pdf_sign_appearance *);
typedef void (*z_sign_drop_appearance_fn)(fz_context*, z_pdf_sign_appearance *);

typedef void (*z_sign_setsign_value_fn)(fz_context*, z_device *);
typedef void (*z_sign_releasedevice_fn)(fz_context*, z_device *);

struct z_pdf_sign_appearance_s {
    int refcount;
    void *app;
    char *text;
    z_sign_set_appearance_fn  set_app;
    z_sign_keep_appearance_fn keep_app;
    z_sign_drop_appearance_fn drop_app;
};

struct z_device_s {
    int refcount;
    z_sign_signature_fn       do_sign;
	z_sign_get_digest_fn      get_digest;
    z_sign_releasedevice_fn   release;
};

z_device *z_keep_device(fz_context *ctx, z_device *device);
void z_drop_device(fz_context *ctx, z_device *device);

void z_pdf_drop_sign_appreance(fz_context *ctx, z_pdf_sign_appearance *app);
z_pdf_sign_appearance *z_pdf_new_image_sign_appearance(fz_context *ctx, fz_image *image, char *text);

void z_pdf_dosign(fz_context *ctx, z_device *device, pdf_document *doc,int pageno, fz_rect rect, z_pdf_sign_appearance *app);

void z_pdf_write_sign(fz_context *ctx, char *file, fz_buffer *buf, int ofs, int len);
void z_fz_stream_save(fz_context *ctx, fz_stream *stm, char *filename);



#endif
