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
#if 0
fz_buffer *z_points_to_PdfScriptStream(fz_context *ctx,pdf_document *doc,
       pdf_page *page, z_points *points) 
{
    if( !ctx || !doc || !points || 0==points->count )
        return NULL;

    const char *header =  "q\n"                     \
        "1.000 0.000 0.000 -1.000 0.000 %.3f cm\n"  \
        "1 J 1 j /DeviceRGB CS 0.00 0.00 0.00 SCN\n";
    const char *item = "%.2f w\n %.2f %.2f m\n%.2f %.2f l\nS\nf\n";

    fz_rect r;
    pdf_bound_page(ctx, page, &r);
    float h = r.y1 - r.y0;
    pdf_drop_page(ctx, page); page = NULL; 

    int bufsize = 128 + (points->count * 64);
    char *data = (char*)fz_malloc(ctx, bufsize);
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

int pdf_draw_pointsToPage(fz_context *ctx, pdf_document *doc, z_points *points,
    int pageno){
    pdf_page *page = NULL;
    fz_buffer *buffer = NULL;
    int ret = z_error;
    fz_try(ctx) {
        page = pdf_load_page(ctx,doc, pageno); 
        buffer = z_points_to_PdfScriptStream(ctx,doc, page, points);
        if( !buffer )
            return z_error;
        // pdf_obj *pageobj = pdf_lookup_page_obj(ctx, doc, pageno);
        ret = pdf_add_content_Stream(ctx, doc, page->me, buffer);
    }
	fz_always(ctx)
	{
        if(buffer) fz_drop_buffer(ctx, buffer);
        if(page) pdf_drop_page(ctx, page);
	}
    fz_catch(ctx) {
        printf("%s,message:%s\n", "error in pdf_draw_pointsToPage", 
                ctx->error->message);
    }
    return ret;
} 
#endif
//annot = pdf_create_annot(ctx, idoc, (pdf_page *)page, FZ_ANNOT_INK);
//pdf_set_ink_annot_list(ctx, idoc, annot, pts, counts, n, color, INK_THICKNESS);
//fz_maxtrix ctm;
//fz_invert_matrix(&ctm, &annot->page->ctm);
//fz_transform_point(&pt, &ctm);
//pdf_page *page = pdf_load_page(ctx, doc, pageno);
//page->ctm



























