/*
 * =====================================================================================
 *
 *       Filename:  z_pdf_ntko_data.c
 *
 *    Description:  jjjj
 *
 *        Version:  1.0
 *        Created:  2017/03/15 17时59分39秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
// ntko defined annotaion data
// to store ntko custom data
// Object structure:
// ......annotation begin.....
// <</Type /Annotation 
//      /NTKO
//      <<  /Password /password_md5_string
//          /Data /data_string
//      >>
// ......the other element in Annotaion
// >>

#include "mupdf/pdf.h"

#define NTKO_OBJ_NAME_NTKO          "NTKO"
#define NTKO_OBJ_NAME_Password      "Passowrd"
#define NTKO_OBJ_NAME_Data          "Data"

pdf_obj*
z_pdf_new_ntko_data_object(fz_context *ctx, pdf_document *doc, const char *password, const char *data) 
{
    pdf_obj *ntkoobj = NULL;
    fz_try(ctx) {
    if(password) {
        ntkoobj = pdf_new_dict(ctx, doc, 1);
        pdf_dict_puts_drop(ctx, ntkoobj, NTKO_OBJ_NAME_Password,
                pdf_new_string(ctx, doc, password, strlen(password)));
    }

    if(data) {
        if(!ntkoobj)
            ntkoobj  = pdf_new_dict(ctx, doc, 1); 
        pdf_dict_puts_drop(ctx, ntkoobj, NTKO_OBJ_NAME_Data,
                pdf_new_string(ctx, doc, data, strlen(data)));
    }
    }
    fz_catch(ctx) {
        fz_rethrow(ctx);
    }
    return ntkoobj;
}

void
z_pdf_annot_put_data(fz_context *ctx, pdf_annot *annot, const char *password, const char *data)
{
    pdf_obj *ntkoobj = NULL;
    fz_try(ctx) {
        ntkoobj = z_pdf_new_ntko_data_object(ctx, annot->page->doc, password, data);
        if(ntkoobj)
            pdf_dict_puts_drop(ctx, annot->obj, NTKO_OBJ_NAME_NTKO, ntkoobj);
    }
    fz_catch(ctx) {
        if(ntkoobj) pdf_drop_obj(ctx, ntkoobj);
        fz_rethrow(ctx);
    } 
}

const char *
z_pdf_get_ntko_data_string(fz_context *ctx, pdf_obj *obj, const char* type) {
    pdf_obj *ntkoobj = NULL;
    pdf_obj *subobj = NULL; 
    const char *value = NULL;

    fz_try(ctx) {
        ntkoobj = pdf_dict_gets(ctx, obj, NTKO_OBJ_NAME_NTKO);
        if(ntkoobj) {
            subobj = pdf_dict_gets(ctx, ntkoobj, type); 
            value = pdf_to_str_buf(ctx, subobj);
        }
    }
    fz_catch(ctx) {
        fz_warn(ctx, "%s", fz_caught_message(ctx));
    }

    return value; 
}

const char* 
z_pdf_ntko_password(fz_context *ctx, pdf_obj *obj) 
{
    return z_pdf_get_ntko_data_string(ctx, obj, NTKO_OBJ_NAME_Password);
}

const char* 
z_pdf_ntko_data(fz_context *ctx, pdf_obj *obj) 
{
    return z_pdf_get_ntko_data_string(ctx, obj, NTKO_OBJ_NAME_Data);
}


