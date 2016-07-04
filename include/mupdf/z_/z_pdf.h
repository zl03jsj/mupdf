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
#include "mupdf/pdf.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    z_okay = 0,
    z_error = 1
} Z_ERROR_CODE;

#include "z_algorithm.h"
#include "pdf_extension.h"
//________________________pdf_add_contentstream.c
int pdf_draw_pointsToPage(fz_context* ctx, pdf_document *doc, z_points *points, int pageno);

#ifdef __cplusplus
}
#endif

#endif // #define Z_Pdf_h
