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
// #define RES_Image_file  RES_PATH"/Monkey_D_Luffey.png"
// #define RES_Image_file  RES_PATH"/esp_saved.png"
// #define RES_Image_file  "/Users/zl03jsj/Downloads/pdf-pre-post-annotation.png"
#define RES_Image_file  RES_PATH"/esp_saved.png"

// #define RES_Pdf_file    RES_PATH"/pdffile/PDF32000_2008.pdf"
#define RES_Pdf_file    RES_PATH"/pdffile/test.pdf"
#define RES_Cert_file   RES_PATH"/user/zl.pfx"
#define RES_CA_file     RES_PATh"/ca/zl_ca.pfx"
#define RES_Pdf_savedfile RES_PATH"/saved.pdf"
#define Defualt_password "111111"

void testAddAnnotation(fz_context *ctx, pdf_document *doc, int pageno, char *savefile) {
    z_fpoint_array *fps = NULL;
    z_fpoint_arraylist *fpal = NULL;
    z_pdf_sign_appearance *app = NULL;
    pdf_page *page = NULL;
    fz_try (ctx)  {
        fz_matrix mtx;
        float zoom = 72.0 / 200;
        fz_scale(&mtx, zoom, zoom);
        page = pdf_load_page(ctx, doc, pageno);
        z_fpoint *fp = NULL;
        fps = z_new_fpoint_array(ctx, 45, 5.0f, 0.5f);
        fp = fps->point;
        fp->p.x = 776.703; fp->p.y = 628.713; fp->w = 0.5;       fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 774.700; fp->p.y = 628.553; fp->w = 0.5415687; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 772.697; fp->p.y = 628.392; fp->w = 0.5831374; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 770.694; fp->p.y = 628.232; fp->w = 0.6247061; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 768.691; fp->p.y = 628.072; fp->w = 0.6662748; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 768.691; fp->p.y = 628.072; fp->w = 0.6662748; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 767.022; fp->p.y = 628.047; fp->w = 0.6756242; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 765.222; fp->p.y = 628.228; fp->w = 0.6849737; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 763.289; fp->p.y = 628.616; fp->w = 0.6943232; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 761.225; fp->p.y = 629.210; fp->w = 0.7036726; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 759.028; fp->p.y = 630.011; fp->w = 0.7130221; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 756.699; fp->p.y = 631.018; fp->w = 0.7223716; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 754.239; fp->p.y = 632.231; fp->w = 0.7317210; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 751.646; fp->p.y = 633.651; fp->w = 0.7410705; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 748.921; fp->p.y = 635.277; fp->w = 0.7504200; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 748.921; fp->p.y = 635.277; fp->w = 0.7504200; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 745.676; fp->p.y = 637.825; fp->w = 0.7268964; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 742.927; fp->p.y = 640.863; fp->w = 0.7033730; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 740.675; fp->p.y = 644.391; fp->w = 0.6798495; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 738.920; fp->p.y = 648.409; fp->w = 0.6563260; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 737.661; fp->p.y = 652.917; fp->w = 0.6328025; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 736.900; fp->p.y = 657.916; fp->w = 0.6092790; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 736.635; fp->p.y = 663.405; fp->w = 0.5857556; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 736.867; fp->p.y = 669.384; fp->w = 0.5622321; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 737.596; fp->p.y = 675.854; fp->w = 0.5387085; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 737.596; fp->p.y = 675.854; fp->w = 0.5387085; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 739.716; fp->p.y = 684.186; fp->w = 0.5061289; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 742.637; fp->p.y = 691.989; fp->w = 0.4735494; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 746.359; fp->p.y = 699.264; fp->w = 0.4409698; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 750.882; fp->p.y = 706.010; fp->w = 0.4083902; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 756.206; fp->p.y = 712.226; fp->w = 0.3758106; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 762.331; fp->p.y = 717.914; fp->w = 0.3432310; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 769.257; fp->p.y = 723.073; fp->w = 0.3106514; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 776.984; fp->p.y = 727.703; fp->w = 0.2780718; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 785.513; fp->p.y = 731.803; fp->w = 0.2454922; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 785.513; fp->p.y = 731.803; fp->w = 0.2454922; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 796.804; fp->p.y = 735.406; fp->w = 0.2431225; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 807.487; fp->p.y = 738.171; fp->w = 0.2407528; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 817.562; fp->p.y = 740.098; fp->w = 0.2383831; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 827.028; fp->p.y = 741.187; fp->w = 0.2360134; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 835.887; fp->p.y = 741.437; fp->w = 0.2336438; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 844.136; fp->p.y = 740.850; fp->w = 0.2312741; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 851.778; fp->p.y = 739.424; fp->w = 0.2289044; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 858.811; fp->p.y = 737.161; fp->w = 0.2265347; fz_transform_point(&fp->p, &mtx);fp ++;
        fp->p.x = 865.236; fp->p.y = 734.059; fp->w = 0.2241650; fz_transform_point(&fp->p, &mtx);fp ++;

        fps->len = 45;

        fpal = z_new_fpoint_arraylist(ctx);
        z_fpoint_arraylist_append(ctx, fpal, fps);

        app = z_pdf_new_sign_appearance_with_paths(ctx, fpal, fz_empty_rect, NULL);
        z_pdf_add_annotation(ctx, page, app, "password", "ntkodata!!!!!!!!!!!!!!!!!!");
//        pdf_write_options opts = {0};
//        pdf_save_document(ctx, doc, savefile, &opts);
        z_pdf_incremental_save_document(ctx, doc, savefile, NULL);
    }
    fz_always(ctx) {
        if(fps) z_drop_fpoint_array(ctx, fps);
        if(fpal) z_drop_fpoint_arraylist(ctx, fpal);
        if(app) z_pdf_drop_sign_appreance(ctx, app);
        if(page) pdf_drop_page(ctx, page);
    }
    fz_catch(ctx) {
        fz_warn(ctx, "%s", fz_caught_message(ctx));
    }
}

#if 1
void doTestPdfSign(fz_context *ctx, pdf_document *doc, int pageno, fz_rect rect, char *savefile) 
{
    printf("dosign test pdf");
    z_device * device = z_openssl_new_device(ctx, RES_PATH"/user/zl.pfx", "111111");
    fz_image * image = fz_new_image_from_file(ctx, RES_Image_file); 
    z_pdf_sign_appearance *app = z_pdf_new_sign_appearance_with_image(ctx, image, rect, NULL);
    
    pdf_page *page = pdf_load_page(ctx, doc, pageno);
    z_pdf_dosign_with_page(ctx, device, doc, page, app);
    pdf_drop_page(ctx, page);
    // z_pdf_dosign(ctx, device, doc, pageno, app);

    z_drop_device(ctx, device);
    z_pdf_incremental_save_document(ctx, doc, savefile, NULL);
}
#else
void doTestsign(fz_context *ctx, pdf_document *doc, char *ofilename, fz_rect r) {
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
#endif

void doTestAddImage(fz_context *ctx, pdf_document *doc)
{
    fz_try(ctx) {
        pdf_add_imagefile(ctx, doc, RES_Image_file, 0, 0, 0, 100, 100);
        z_pdf_incremental_save_document(ctx, doc, RES_Pdf_savedfile, NULL);
    }
    fz_catch(ctx) {
        fz_warn(ctx, "%s", fz_caught_message(ctx));
    }

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

static void printMatrix(fz_matrix *mx)
{
    printf("matrix:\n\t[%f,%f]\n\t[%f,%f]\n\t[%f,%f]\n", mx->a, mx->b, mx->c, mx->d, mx->e, mx->f);
}

void matrixTest() {
    fz_matrix mx1 = {1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f};  // x 对称
    fz_matrix mx2 = {1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 841.9f};// 
    fz_matrix r = fz_identity;
    fz_concat(&r, &mx1, &mx2);
    printMatrix(&r);
}
int main(int argc, char **argv) {

    printf("main is call\n");
    // matrixTest();
    char *infile, *ofile;
    // fz_rect r = {108.77f, 47.03f, 226.77f, 165.03f};
    // fz_rect r = {208.77f, 47.03f, 326.77f, 165.03f};
    fz_rect r = {50.0f, 50.0f, 150.0f, 150.0f};
    int w, h;
    w = r.x1 - r.x0;
    h = r.y1 - r.y0;
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
    // doTestPdfSign(ctx, doc, 0, r, ofile);
    // doTestAddImage(ctx, doc);
    testAddAnnotation(ctx, doc, 0, ofile);
    pdf_drop_document(ctx, doc);
    fz_drop_context(ctx);
    printf("====================\n");
    printf("infile:%s\n", infile);
    printf("outfile:%s\n", ofile);
    return 1;
}






