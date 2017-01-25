/*
 * =====================================================================================
 *
 *       Filename:  z_sign.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/17/2016 17:33:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include "mupdf/pdf.h"

z_device *z_keep_device(fz_context *ctx, z_device *device)
{
    device->refcount++;
    return device;
}

void z_drop_device(fz_context *ctx, z_device *device)
{
    if(!ctx || !device) return;

    if(0==(--device->refcount) && device->release)
    {
        device->release(ctx, device);
    }
}

void z_pdf_dosign_with_page(fz_context *ctx, z_device *device, pdf_document *doc,pdf_page *page, z_pdf_sign_appearance *app) {
    pdf_annot *annot = NULL;
    char *signame = NULL;
    fz_try(ctx) {
        if(!page)
			fz_throw(ctx, FZ_ERROR_GENERIC, "invalid page.");
        signame = new_unique_string(ctx, "ntkosign_", NULL);
        // doc->disallow_new_increments = 1;
        annot = (pdf_annot*)pdf_create_widget(ctx, doc, page,  PDF_WIDGET_TYPE_SIGNATURE, signame); 
        // must add /P(which page the annot belong to) tag, or signature object cannot be display on
        // adobe reader's signature panel
        pdf_dict_put_drop(ctx, annot->obj, PDF_NAME_P, pdf_new_indirect(ctx, doc, pdf_obj_parent_num(ctx, page->obj), 0));
        // pdf_annot_rect
//        annot->pagerect = app->rect;
//        annot->rect = app->rect;
        // fz_transform_rect(&annot->pagerect, &annot->page->ctm);
        // pdf_dict_puts_drop(ctx, annot->obj, "Rect", pdf_new_rect(ctx, doc, &annot->pagerect));
        doc->disallow_new_increments = 1;
        device->do_sign(ctx, device, doc, annot, app);
    }
	fz_always(ctx) {
        if(signame) fz_free(ctx, signame);
	}
	fz_catch(ctx) {
        if(page) pdf_drop_page(ctx, page);
        if(annot) pdf_drop_annots(ctx, annot);
		fz_rethrow(ctx);
	}
}
void z_pdf_dosign(fz_context *ctx, z_device *device, pdf_document *doc,int pageno, z_pdf_sign_appearance *app)
{
    pdf_page *page = NULL;
    pdf_annot *annot = NULL;
    char *signame = NULL;
    fz_try(ctx) {
        page = pdf_load_page(ctx, doc, pageno);
        if(!page)
			fz_throw(ctx, FZ_ERROR_GENERIC, "Failed to load page:%d", pageno);

        signame = new_unique_string(ctx, "ntkosign_", NULL);
        // doc->disallow_new_increments = 1;
        annot = (pdf_annot*)pdf_create_widget(ctx, doc, page,  PDF_WIDGET_TYPE_SIGNATURE, signame); 
        // must add /P(which page the annot belong to) tag, or signature object cannot be display on
        // adobe reader's signature panel
        pdf_dict_put_drop(ctx, annot->obj, PDF_NAME_P, pdf_lookup_page_obj(ctx, doc, pageno));

//        annot->pagerect = app->rect;
//        annot->rect = app->rect;
        // fz_transform_rect(&annot->pagerect, &annot->page->ctm);
        // pdf_dict_puts_drop(ctx, annot->obj, "Rect", pdf_new_rect(ctx, doc, &annot->pagerect));
        doc->disallow_new_increments = 1;
        device->do_sign(ctx, device, doc, annot, app);
    }
	fz_always(ctx) {
        if(signame) fz_free(ctx, signame);
	}
	fz_catch(ctx) {
        if(page) pdf_drop_page(ctx, page);
        if(annot) pdf_drop_annots(ctx, annot);
		fz_rethrow(ctx);
	}
}

z_pdf_sign_appearance *z_pdf_new_sign_appearance_with_image(fz_context *ctx, fz_image *image, fz_rect r, char *text)
{
    z_pdf_sign_appearance *app = NULL;
    fz_try(ctx) {
        app = fz_malloc_struct(ctx, z_pdf_sign_appearance);
        app->refcount = 1;
        app->set_app = z_pdf_set_signature_appearance_with_image;
        app->keep_app = (z_sign_keep_appearance_fn)fz_keep_image;
        app->drop_app = (z_sign_drop_appearance_fn)fz_drop_image;

        app->app = fz_keep_image(ctx, image);
        app->rect = r;
		app->text = text==NULL?NULL:fz_strdup(ctx, text);
    }
    fz_always(ctx) { 
    }
    fz_catch(ctx) { 
        z_pdf_drop_sign_appreance(ctx, app);
    }
    return app;
}

z_pdf_sign_appearance *z_pdf_new_sign_appearance_with_paths(fz_context *ctx, z_fpoint_arraylist *al, fz_rect r,char *text)
{
    z_pdf_sign_appearance *app = NULL;
    fz_try(ctx) {
        app = fz_malloc_struct(ctx, z_pdf_sign_appearance);
        app->refcount = 1;
        app->set_app = z_pdf_set_signature_appearance_with_path;
        app->keep_app = (z_sign_keep_appearance_fn)z_keep_fpoint_arraylist;
        app->drop_app = (z_sign_drop_appearance_fn)z_drop_fpoint_arraylist;

        app->app = z_keep_fpoint_arraylist(ctx, al);
        app->rect = r;
		app->text = text==NULL?NULL:fz_strdup(ctx, text);
    }
    fz_always(ctx) { 
    }
    fz_catch(ctx) { 
        z_pdf_drop_sign_appreance(ctx, app);
    }
    return app;
}

z_pdf_sign_appearance *z_pdf_keep_sign_apperance(fz_context *ctx, z_pdf_sign_appearance *app)
{
    app->refcount++;
    return app;
}

void z_pdf_drop_sign_appreance(fz_context *ctx, z_pdf_sign_appearance *app)
{
    app->refcount--;
    if( 0==app->refcount ) {
        if(app->app && app->drop_app)
            app->drop_app(ctx, app->app);

        if(app->text)
            fz_free(ctx, app->text);

        fz_free(ctx, app);
    }
}

void z_pdf_write_sign(fz_context *ctx, char *file, fz_buffer *buf, int ofs, int len)
{
    FILE *f = NULL;
    unsigned char * d;
    size_t size;
    fz_try(ctx) {
        if(!buf) 
            fz_throw(ctx, FZ_ERROR_GENERIC, "Insufficient space for digest");
        size = fz_buffer_extract(ctx, buf, &d);
        if( (int)size+2 > len )
            fz_throw(ctx, FZ_ERROR_GENERIC, "Insufficient space for digest");
        f = fz_fopen(file, "rb+");
        if (f == NULL)
            fz_throw(ctx, FZ_ERROR_GENERIC, "Failed to write digest");

        fseek(f, ofs, SEEK_SET);
        for (unsigned int i = 0; i<size ; i++)
            fprintf(f, "%02x", *(d+i));
    }
    fz_always(ctx) {
        if(f) fclose(f);
    }
    fz_catch(ctx) {
        fz_rethrow(ctx); 
    }
}

void z_fz_stream_save(fz_context *ctx, fz_stream *stm, char *filename)
{
    fz_output *o = NULL;
    int old_ofs = 0;
    unsigned char buf[1024];

    fz_try(ctx) {
        int readcount = 0;
        old_ofs = fz_tell(ctx, stm);
        o = fz_new_output_with_path(ctx, filename, 0);
        fz_seek(ctx, stm, 0, SEEK_SET);
        while(0 != (readcount=fz_read(ctx, stm, buf, 1024)) ) {
            fz_write(ctx, o, buf, readcount); 
        }
    }
    fz_always(ctx) {
        fz_seek(ctx, stm, old_ofs, SEEK_SET);
        fz_drop_output(ctx, o); 
    }
    fz_catch(ctx) {
        fz_rethrow(ctx); 
    }
}
