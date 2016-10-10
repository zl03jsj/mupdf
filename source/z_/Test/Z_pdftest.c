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

#define RES_PATH "/Users/zl03jsj/Documents/pdftest"
#define RES_Image_file  RES_PATH"/monkeysmile.jpg"
// #define RES_Pdf_file    RES_PATH"/pdffile/PDF32000_2008.pdf"
#define RES_Pdf_file    RES_PATH"/pdffile/test.pdf"
#define RES_Cert_file   RES_PATH"/user/zl.pfx"
#define RES_CA_file     RES_PATh"/ca/zl_ca.pfx"
#define RES_Pdf_savedfile RES_PATH"/saved.pdf"
#define Defualt_password "111111"

void Z_PdfSignCtxDisplay(Z_PdfSignContext *signctx);

void doTestsign(fz_context *ctx, pdf_document *doc, char *ofilename, fz_rect r)
{
    Z_sign_device *signdevice = Z_openssl_SignDev_new(ctx, RES_PATH"/user/zl.pfx", "111111");
    fz_image *image = fz_new_image_from_file(ctx, RES_Image_file);
    Z_PdfSignContext *signctx = Z_PdfSignCtxInit(ctx, doc, 0, 1, image, r);
    Z_pdf_add_sign(signctx, signdevice); 
    Z_PdfSignCtxDisplay(signctx);
    fz_drop_image(ctx, image);

    Z_PdfSignCtxClear(signctx);
    Z_signdev_drop(signdevice, ctx);
    pdf_save_incremental_tofile(ctx, doc, ofilename);
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

static void displayUsage(void)
{
	fprintf(stderr,
        "usage: subdata ofs size filename [-o outfile]\n"   
        "\t-o\tname of subdata to store\n"
		);
	exit(1);
}

static void subdata(char *infile, int ofs, int size, char *ofile)
{
    fz_stream *stm = NULL;
    fz_buffer *buf = NULL;
    unsigned char *obuf = NULL;
    fz_context *ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
    fz_try(ctx) {
        stm = fz_open_file(ctx, infile);
        fz_seek(ctx, stm, 0, SEEK_END);
        size = fz_min(fz_tell(ctx, stm)-ofs, size);
        obuf = fz_malloc(ctx, size);

        fz_seek(ctx, stm, ofs, SEEK_SET);
        fz_read (ctx, stm, obuf, size);
        buf = fz_new_buffer_from_data(ctx, obuf, size);

        // or append buffer
        fz_save_buffer(ctx, buf, ofile);
        printf("wirte to file:%s\n", ofile);
    }
    fz_always(ctx) {
        fz_drop_stream(ctx, stm);

        if(buf) fz_drop_buffer(ctx, buf);
        else if(obuf){
            fz_free(ctx, obuf);
        }
    }
    fz_catch(ctx) {
        fprintf(stderr, ctx->error->message);
    }
    fz_drop_context(ctx);
}

static void main_subdata(int argc, char **argv)
{
    char *infile, *ofile; // = "out.subdata";
    int ofs, size, c;

    if( argc<4 ){
        displayUsage();
    }
    ofs  = atoi(argv[1]);
    size = atoi(argv[2]);
    infile = argv[3];
    ofile = "out.subdata";

    fz_optind = 4;

    while ((c = fz_getopt(argc, argv, "o:")) != -1) {
        switch (c)
        {
            case 'o': ofile = fz_optarg; break;
            default:displayUsage(); break;
        }
    }
    subdata(infile, ofs, size, ofile);
}

int main(int argc, char **argv) {
    char *infile, *ofile;
    fz_rect r = {200, 200, 300, 300};// {75, 733, 180, 809};
    int w, h;
    w = 100; h = 100;
    infile = ofile = NULL;
    fz_optind = 1;
    char c;
    while ((c = fz_getopt(argc, argv, "i:o:x:y:w:h:")) != -1) {
        switch (c)
        {
            case 'i': infile = fz_optarg;   break;
            case 'o': ofile  = fz_optarg;    break;
            case 'x': r.x0 = atoi(fz_optarg);  break;
            case 'y': r.y0 = atoi(fz_optarg);  break;
            case 'w': w = atoi(fz_optarg);  break;
            case 'h': h = atoi(fz_optarg);  break;
        }
    }
    r.x1 = r.x0 + w;
    r.y1 = r.y0 + h;

    if(!infile) infile = RES_Pdf_file;
    if(!ofile) ofile = RES_Pdf_savedfile;

    fz_context *ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
    pdf_document *doc = pdf_open_document(ctx, infile);
    // xref = pdf_get_xref_entry
    doTestsign(ctx, doc, ofile, r);
    pdf_drop_document(ctx, doc);
    fz_drop_context(ctx);
    printf("====================\n");
    printf("infile:%s\n", infile);
    printf("outfile:%s\n", ofile);
    return 1;
}






