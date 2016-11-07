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

#include "mupdf/pdf.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    z_okay  = 0,
    z_error = 1
} Z_error_code;

#include "z_algorithm.h"
#include "pdf_extension.h"

#define fz_rect_dx(r) ((r)->x1 - (r)->x0)
#define fz_rect_dy(r) ((r)->y1 - (r)->y0)

//________________________pdf_add_contentstream.c
extern const char *ntkoextobjname;
typedef struct Z_pdf_SignContext_s Z_PdfSignContext;
typedef struct Z_sign_device_context_s Z_sign_device;

fz_buffer *z_points_to_PdfScriptStream(fz_context *ctx,pdf_document *doc, pdf_page *page, z_points *points);
int pdf_draw_pointsToPage(fz_context* ctx, pdf_document *doc, z_points *points, int pageno);



// pdf assist funcs
void Z_pdf_obj_display(fz_context *ctx, pdf_obj *obj);

// pdf signature funcs by zl!!________________
int Z_pdf_signatures_supported(fz_context *ctx);
Z_sign_device *Z_openssl_SignDev_new(fz_context *ctx, const char *file, const char *pw);
int Z_signdev_drop(Z_sign_device *dev, fz_context *ctx);
Z_sign_device *Z_signdev_keep(Z_sign_device *dev, fz_context *ctx);
void Z_pdf_signComplete(Z_sign_device *signDev, fz_context *ctx,
        pdf_document *doc, const char *filename, pdf_obj *byterange,
        int ofs, int size);
fz_buffer *Z_sign(Z_sign_device *dev, fz_context *ctx, pdf_document *doc,
        const char *filename, pdf_obj *byterange);
void Z_writeSign(fz_context *ctx, const char*file, fz_buffer *buff, int ofs, int size);

Z_PdfSignContext *Z_PdfSignCtxInit(fz_context *ctx, pdf_document *doc, 
        int pageno, int stmtype, void *stm, fz_rect r);
int Z_PdfSignCtxClear(Z_PdfSignContext *signCtx);
int Z_pdf_add_sign(Z_PdfSignContext *signctx, Z_sign_device *sigdev);
// void Z_InitOpenSSL(fz_context *ctx);

#if defined(HAVE_OPENSSL)
#include "openssl/bio.h"
BIO *Z_file_segment_BIO(fz_context *ctx, const char *filename, pdf_obj *byterange);
#endif

#ifdef __cplusplus
}
#endif

#endif // #define Z_Pdf_h
