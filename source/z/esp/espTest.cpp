/*
 * =====================================================================================
 *       Filename:  espTest.cpp
 *    Description:  
 *        Version:  1.0
 *        Created:  2017/01/19 16时42分29秒
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  YOUR NAME (), 
 *   Organization:  
 * =====================================================================================
 */
#include "mupdf/z/ntko_esp.h"

int main(int argc, char const *argv[])
{
    const char *espfile = "/Users/zl03jsj/Documents/mupdf/source/z/esp/esp.file";
    fz_context *ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
    NTKOEspParser *espParser = NTKOEspParser::create(ctx); 
    //*
    bool isok = false;
    fz_try(ctx) {
        isok = espParser->open(espfile, (char*)"111111");
        if(isok) {
            const NTKOEspHeader *header = espParser->getHeader();
            fz_image *img = espParser->getImage(); 
            printf("open esp file:%s ok!!\n", espfile);
            printf("ntko esp file header:\n");
            printf("flag: %s\n", header->flag);
            printf("version: %d\n", header->version);
            printf("codepage:%d\n", header->codepage);
            printf("crcVal:  %d\n", header->crcValue);
            printf("signname:%s\n", header->signname);
            printf("signuser:%s\n", header->signuser);
            printf("md5Val:  %s\n", header->md5Val);
            printf("signSN:  %s\n", header->signSN);
            printf("reserved: %s\n", header->reserved);
            printf("---------------------------\n");
            header = NULL;

            fz_pixmap *pixmap = fz_get_pixmap_from_image(ctx, img, NULL, NULL, 0, 0); 
            const char *imagefile = "/Users/zl03jsj/Documents/pdftest/esp_saved.png";
            fz_save_pixmap_as_png(ctx, pixmap, imagefile);
            printf("get esp image,save to:%s\n", imagefile);

            fz_drop_pixmap(ctx, pixmap);
            fz_drop_image(ctx, img); 
        }
        else {
            printf("can't open esp file:%s.", espfile); 
        }
    }
    fz_catch(ctx) {
        fz_warn(ctx, "%s", fz_caught_message(ctx));
    }
    // */
    delete espParser;
    fz_drop_context(ctx);

    return 0;
}
