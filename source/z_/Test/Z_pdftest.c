/*
 * =====================================================================================
 *       Filename:  Z_pdftest.c
 *    Description:
 *        Version:  1.0
 *        Created:  08/01/2016 14:13:58
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include "mupdf/pdf.h"

#define RES_PATH "/Users/zl03jsj/Documents/mupdf/source/z_/Test"
#define RES_Image_file  RES_PATH"/monkeysmile.jpg"
#define RES_Pdf_file    RES_PATH"/pdffile/helloworld.pdf"
#define RES_Cert_file   RES_PATH"/user/zl.pfx"
#define RES_CA_file     RES_PATh"/ca/zl_ca.pfx"
#define RES_Pdf_savedfile RES_PATH"/pdffile/saved.pdf"
#define Defualt_password "111111"

void Z_PdfSignCtxDisplay(Z_PdfSignContext *signctx);

int main(int argc, char **argv) {
    fz_context *ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
    pdf_document *doc = pdf_open_document(ctx, RES_Pdf_file);

    fz_rect r = { 0, 0, 100, 100};
    fz_image *image = fz_new_image_from_file(ctx, RES_Image_file);
    Z_PdfSignContext *signctx = Z_PdfSignCtxInit(ctx, doc, 0, 1, image, r);
    Z_pdf_add_sign(signctx); 
    Z_PdfSignCtxDisplay(signctx);
    fz_drop_image(ctx, image);

    Z_PdfSignCtxClear(signctx);
#if 0
    pdf_page *page = pdf_load_page(ctx, doc, 0);
    pdf_obj *pageobj = page->me;
    pdf_obj *resobj  = page->resources;
	pdf_obj *subres;

    // add ext state obj
	pdf_obj * egsobj = pdf_add_extstate(ctx, doc);
	pdf_resource_add_extgstate(ctx, doc, resobj, ntkoextobjname, egsobj);

    // add image
    fz_image *image = fz_new_image_from_file(ctx, RES_Image_file);
    pdf_init_resource_tables(ctx, doc);
    pdf_obj *imageobj = pdf_add_image(ctx, doc, image, 0);
    fz_drop_image(ctx, image);


	subres = pdf_dict_get(ctx, resobj, PDF_NAME_XObject);
	if (!subres) {
		subres = pdf_new_dict(ctx, doc, 10);
		pdf_dict_put_drop(ctx, resobj, PDF_NAME_XObject, subres);
	}
	pdf_dict_puts_drop(ctx, subres, "zlimage", imageobj);

    // add contents
    pdf_obj *imagecontentobj = pdf_add_content(ctx, doc, "zlimage",
            0, 0, 200, 200);

    pdf_obj *contentobj = page->contents;
    if( !pdf_is_array(ctx, contentobj) ) {
        pdf_obj *arr = pdf_new_array(ctx, doc, 2);
        pdf_array_push(ctx, arr, contentobj);
        pdf_dict_del(ctx, pageobj, contentobj);
        pdf_dict_put_drop(ctx, pageobj, PDF_NAME_Contents, arr);
        contentobj = arr;
    }
    pdf_array_push_drop(ctx, contentobj, imagecontentobj);
    page->contents = contentobj;

    pdf_save_incremental_tofile(ctx, doc, RES_Pdf_savedfile);
    // printf("page obj ref=%d\n", pageobj->refs);
//    printf("res obj ref=%d\n", resobj->refs);
//    printf("egsobj ref=%d\n", egsobj->refs);
//    printf("image ref=%d\n", image->refs);
//    printf("image obj ref=%d\n", imageobj->refs);
//    printf("content obj ref=%d\n", contentobj->refs);
    printf("page ref=%d\n", page->super.refs);
    pdf_drop_page(ctx, page);
#endif
    pdf_save_incremental_tofile(ctx, doc, RES_Pdf_savedfile);
    pdf_drop_document(ctx, doc);
    fz_drop_context(ctx);
    return 1;
}





