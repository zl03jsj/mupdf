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

#define RES_PATH "/Users/zl03jsj/Documents/pdf_test__"
#define RES_Image_file  RES_PATH"/monkeysmile.jpg"
// #define RES_Pdf_file    RES_PATH"/pdffile/PDF32000_2008.pdf"
#define RES_Pdf_file    RES_PATH"/pdffile/Helloworld.pdf"
#define RES_Cert_file   RES_PATH"/user/zl.pfx"
#define RES_CA_file     RES_PATh"/ca/zl_ca.pfx"
#define RES_Pdf_savedfile RES_PATH"/saved.pdf"
#define Defualt_password "111111"

void Z_PdfSignCtxDisplay(Z_PdfSignContext *signctx);

void doTestsign(fz_context *ctx, pdf_document *doc)
{
    fz_rect r = {95, 703, 220, 853};
    fz_image *image = fz_new_image_from_file(ctx, RES_Image_file);
    Z_PdfSignContext *signctx = Z_PdfSignCtxInit(ctx, doc, 0, 1, image, r);
    Z_pdf_add_sign(signctx); 
    Z_PdfSignCtxDisplay(signctx);
    fz_drop_image(ctx, image);

    Z_PdfSignCtxClear(signctx);
    pdf_save_incremental_tofile(ctx, doc, RES_Pdf_savedfile);
}

void doTestAddImage(fz_context *ctx, pdf_document *doc)
{
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
            75, 650, 200, 200);
    pdf_obj *contentobj = page->contents;
    if( !pdf_is_array(ctx, contentobj) ) {
        pdf_obj *arr = pdf_new_array(ctx, doc, 2);

        pdf_array_push_drop(ctx, arr, imagecontentobj);
        pdf_array_push(ctx, arr, contentobj);

        pdf_dict_del(ctx, pageobj, contentobj);
        pdf_dict_put_drop(ctx, pageobj, PDF_NAME_Contents, arr);
        contentobj = arr;
    }
    else {
        pdf_array_push_drop(ctx, contentobj, imagecontentobj);
    }
    page->contents = contentobj;

    pdf_save_incremental_tofile(ctx, doc, RES_Pdf_savedfile);
    printf("page ref=%d\n", page->super.refs);
    pdf_drop_page(ctx, page);
}

void doTestSaveSigndata(char* infile, int b1, int size1, int b2,int size2, char *ofile)
{
    char buf[256];
    FILE *fs = fopen(infile, "rb");
    FILE *fd = fopen(ofile, "w+b");

    int i, tmp;

    fseek(fs, b1, SEEK_SET);
    i = 0;
    while(i<size1){
        tmp = fread(buf, 1,fz_min(sizeof(buf), size1-i), fs); 
        fwrite(buf, 1, tmp, fd);
        i+=tmp;
    }

    fseek(fs, b2, SEEK_SET);
    i = 0;
    while(i<size2){
        tmp = fread(buf, 1, fz_min(sizeof(buf), size2-i), fs); 
        fwrite(buf, 1, tmp, fd);
        i+=tmp;
    }

    fclose(fs);
    fclose(fd);
}

int main(int argc, char **argv) {
    // doTestSaveSigndata(RES_PATH"/pdffile/saved.pdf", 0, 79957, 84055, 625,
    //      RES_PATH"/signorigin_buff.data");

   fz_context *ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
   pdf_document *doc = pdf_open_document(ctx, RES_Pdf_file);
   // [0 79957 84055 625]
   // doTestAddImage(ctx, doc);
   doTestsign(ctx, doc);
   
   pdf_drop_document(ctx, doc);
   fz_drop_context(ctx);
    return 1;
}





