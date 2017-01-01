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

#ifdef __cplusplus
}
#endif

#endif // #define Z_Pdf_h
