/*
 * =====================================================================================
 *
 *       Filename:  z_pdf.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/02/2016 23:58:36
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef Z_Pdf_h_
#define Z_Pdf_h_

// #pragma message("define HAVE_OPENSSL macro or not")
// #define HAVE_OPENSSL
// #define Z_pdf_sign_

#if 0
#define DO_PRAGMA(x) _Pragma (#x)
#define TODO(x) DO_PRAGMA(message ("TODO - " #x)) 
TODO(Remember to fix this)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "z_math.h"
#include "pdf_extension.h"
#include "mupdf/pdf.h"

typedef enum {
    z_okay  = 0,
    z_error = 1
} Z_error_code;


#define fz_rect_dx(r) ((r)->x1 - (r)->x0)
#define fz_rect_dy(r) ((r)->y1 - (r)->y0)

fz_buffer *z_new_buffer_deflate_data(fz_context *ctx, char *p, size_t n);
fz_buffer *z_deflate_buffer(fz_context *ctx, fz_buffer *bf);

fz_buffer *z_deflate_buffer(fz_context *ctx, fz_buffer *bf);

// ntko custom defined pdf_object, to store ntko custom data
// Object data structure:
// ......annotation begin.....
// <</Type /Annotation 
//      /NTKO
//      <<  /Password /password_md5_string
//          /Data /data_string
//      >>
// ......the other element in Annotaion
// >>
//
/* create new ntko custom pdf object from password, custom data */
pdf_obj* z_pdf_new_ntko_data_object(fz_context *ctx, pdf_document *doc, const char *password, const char *data);

/* add custom pdf_obj to annot */
void z_pdf_annot_put_data(fz_context *ctx, pdf_annot *annot, const char *password, const char *data);

void z_pdf_add_annotation(fz_context *ctx, pdf_page *page, z_pdf_sign_appearance *app, const char *psw, const char *data); 

const char* z_pdf_ntko_password(fz_context *ctx, pdf_obj *obj);

const char* z_pdf_ntko_data(fz_context *ctx, pdf_obj *obj); 

#ifdef __cplusplus
}
#endif

#endif // #define Z_Pdf_h
