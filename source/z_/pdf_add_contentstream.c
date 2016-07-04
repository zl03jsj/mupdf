/*
 * =====================================================================================
 *       Filename:  pdf_add_contentstream.c
 *    Description:  add points array with line width, as content stream to pdf.
 *        Version:  1.0
 *        Created:  07/02/2016 23:48:58
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  zl(88911562@qq.com), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include "mupdf/pdf.h"
#include "mupdf/z_/z_pdf.h"
// 0123456789abcdef0123456789abcdef0123456789abcdef01234567890abcdef  
// header------------------------------------------------------------
// q                                                        2       bytes
// 1.00000 0.00000 0.00000 -1.00000 0.00000 842.00000 cm    56(60)  bytes   
// 1 J 1 j /DoeviceRGB CS 0.00 0.00 0.00 SCN                46      bytes
// //////////////104 count as 128 bytes
// item--------------------------------------------------------------
// 1.22 w                                                   7   bytes
// 197.37 27.00 m                                           15  bytes
// 197.93 29.86 l                                           15  bytes
// S                                                        2   bytes
// f                                                        2   bytes
// //////////////41 count as 64 bytes
static fz_buffer *z_points_to_PdfContentStream(fz_context *ctx,pdf_document *doc,
        int pageno, z_points *points) 
{
    if( !ctx || !doc || !points || 0==points->count ) return NULL;
    int pageCount = pdf_count_pages(ctx, doc);
    if( pageno<0 || pageno>pageCount ) return NULL;

    const char *header =  "q\n"                     \
        "1.000 0.000 0.000 -1.000 0.000 %.3f cm\n"  \
        "1 J 1 j /DeviceRGB CS 0.00 0.00 0.00 SCN\n";
    const char *item = "%.2f w\n %.2f %.2f m\n%.2f %.2f l\nS\nf\n";

    fz_rect r;
    pdf_page *page =  pdf_load_page(ctx, doc, pageno);
    pdf_bound_page(ctx, page, &r);
    float h = r.y1 - r.y0;
    pdf_drop_page(ctx, page); page = NULL; 

    int bufsize = 128 + (points->count * 64);
    unsigned char *data = fz_malloc(ctx, bufsize);
    memset(data, 0, bufsize);

    int pos = 0;
    pos += snprintf(data + pos, bufsize - pos, header, h);
    z_point_width *p = points->data;

    for(int i=0; i<(points->count-1); i++) {
        pos += snprintf(data + pos, bufsize - pos, item, p[i+1].w,
           p[i+0].p.x, p[i+0].p.y,
           p[i+1].p.x, p[i+1].p.y );
    }
	fz_buffer *bfCompressed = deflate_buffer_fromdata(ctx, data, pos); 
    fz_free(ctx, data); data = NULL; 
    return bfCompressed;
}

static int pdf_add_contentStream(fz_context *ctx, pdf_document *doc, int pageno, 
    fz_buffer *buffer)
{
	pdf_obj * page = pdf_lookup_page_obj(ctx, doc, pageno);
    if( !page ) return z_error;
    pdf_obj *obj = pdf_new_dict(ctx, doc, 1);
    pdf_obj *objref = pdf_add_object(ctx, doc, obj);
    pdf_update_stream(ctx, doc, obj, buffer, 1);
    int ret = pdf_page_add_content(ctx, doc, page, objref); 
    if( ret == z_error ) {
        pdf_delete_object(ctx, doc, pdf_to_num(ctx, objref));
        pdf_drop_obj(ctx, obj); 
    }
    return ret;
}

int pdf_draw_pointsToPage(fz_context *ctx, pdf_document *doc, z_points *points, 
    int pageno){
    fz_buffer *buffer = z_points_to_PdfContentStream(ctx,doc, pageno, points);
    if( !buffer )
        return z_error;
    int ret = pdf_add_contentStream(ctx, doc, pageno, buffer);
    fz_drop_buffer(ctx, buffer);
    return ret;
} 
//    annot = pdf_create_annot(ctx, idoc, (pdf_page *)page, FZ_ANNOT_INK);
// pdf_set_ink_annot_list(ctx, idoc, annot, pts, counts, n, color, INK_THICKNESS);
    //fz_maxtrix ctm;
	//fz_invert_matrix(&ctm, &annot->page->ctm);
    //fz_transform_point(&pt, &ctm);
    //pdf_page *page = pdf_load_page(ctx, doc, pageno);
    //page->ctm



























