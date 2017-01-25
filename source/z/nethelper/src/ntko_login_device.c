/*
 * =====================================================================================
 *       Filename:  ntko_login_device.c
 *    Description:  login to ntko sign server
 *        Version:  1.0
 *        Created:  2017/01/24 17时44分20秒
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  YOUR NAME (), 
 *   Organization:  
 * =====================================================================================
 */

#include "net_helper.h"

typedef int  (*login_fun)(ntko_http_helper *helper, nh_login_device *dev);
typedef void (*device_free_fun)(fz_context *ctx, nh_login_device *dev);

// device of login with cert
// to implement.
#if 0
struct nh_login_device_cert_s {
    nh_login_device super;
};
#endif

typedef struct nh_login_device_psw_s nh_login_device_psw; 

struct nh_login_device_s {
    unsigned int ref; 
    char *url;
    nh_submit_method submittype;
    login_fun login;
    device_free_fun freeme;
}; 

void nh_drop_login_device(fz_context *ctx, nh_login_device *dev)
{
    if(0!=(--dev->ref)) return;
    fz_free(ctx, dev->url);
    dev->url= null;
    if(dev->freeme) {
        dev->freeme(ctx, dev); 
    }
    else
        fz_warn(ctx, "freeme member of login device is null");
}

nh_login_device *nh_keep_login_device(fz_context *ctx, nh_login_device *dev)
{
    dev->ref++;
    return dev; 
}

struct nh_login_device_psw_s {
    nh_login_device super;
    u8string username;
    u8string password; 
}; 

static void psw_login_device_free(fz_context *ctx, nh_login_device *dev) 
{
    nh_login_device_psw *psw_dev = (nh_login_device_psw*)dev;
    fz_free(ctx, psw_dev->username);
    fz_free(ctx, psw_dev->password); 
    fz_free(ctx, psw_dev);
}

static int psw_login(ntko_http_helper *hp, nh_login_device *dev) 
{
    nh_login_device_psw *psw_dev = (nh_login_device_psw*)dev;
    fz_context *ctx = nh_get_context(hp);
    http_data *htd = NULL;
    int status = false;

    fz_try(ctx) {
        if(dev->submittype==get)
            htd = nh_do_get(hp, dev->url, null/* dev->username&dev->password */);
        else if(dev->submittype==post)
            htd = nh_do_post(hp, dev->url, null/* dev->username&dev->password */);
        else
            fz_throw(ctx, FZ_NET_ERR_INVALID_PARAMETER, "unkown http submit method");

        if(htd) {
            status = htd->status_code; 
            if(htd->data) 
                fz_drop_buffer(ctx, htd->data);
            fz_free(ctx, htd);
        }
    }
    fz_catch(ctx) {
        fz_rethrow(ctx);
    }
    return status;
}

int nh_login(ntko_http_helper *helper, nh_login_device *dev) 
{
    int status = false;
    fz_context *ctx = nh_get_context(helper);
    if(!helper || !ctx) {
        printf("invalid login parameter");
        return status;
    } 
    fz_try(ctx) {
        status = dev->login(helper, dev);
    }
    fz_catch(ctx)
        fz_rethrow(ctx);

    return status; 
}

nh_login_device *nh_new_login_device_psw(fz_context *ctx, char* url, char *username, char *password) 
{
    nh_login_device_psw *psw_dev = null;
    fz_try(ctx) {
        nh_login_device *super = null;
        psw_dev = fz_malloc_struct(ctx, nh_login_device_psw);
        super = &psw_dev->super;
        super->ref = 1;
        super->submittype = post;
        super->url = fz_strdup(ctx, url);
        super->freeme = psw_login_device_free;
        super->login = psw_login;
        psw_dev->username = fz_strdup(ctx, username);
        psw_dev->password = fz_strdup(ctx, password);
    }
    fz_always(ctx) {
        if(psw_dev) 
            psw_login_device_free(ctx, (nh_login_device*)psw_dev); 
    }
    fz_catch(ctx) {
        fz_rethrow(ctx);
    }

    return &psw_dev->super; 
}
