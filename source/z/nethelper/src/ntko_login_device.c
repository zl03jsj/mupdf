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
#include "ntko_helper.h"

typedef int (*login_fun)(http_helper *helper, nh_login_device *dev, ntko_server_info *svrinfo);
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
    device_free_fun free;
}; 

void nh_drop_login_device(fz_context *ctx, nh_login_device *dev)
{
    if(0!=(--dev->ref)) return;
    fz_free(ctx, dev->url);
    dev->url= null;
    if(dev->free) {
        dev->free(ctx, dev); 
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

static 
int psw_login(http_helper *helper, nh_login_device *dev, ntko_server_info *svrinfo) 
{
    fz_context *ctx = nh_get_context(helper);
    nh_login_device_psw *psw_dev = (nh_login_device_psw*)dev; 
    fz_try(ctx) {
        char data[256];
        char url[256]; 
        char *postdata = null;

        memset(data, 0, sizeof(data));
        fz_snprintf(data, sizeof(data),
            "username=%s&password=%s&clientver=%s&lictype=%d",
            psw_dev->username,
            psw_dev->password,
            "1.0",
            svrinfo->lictype); 

        memset(url, 0, sizeof(url));
        fz_strlcat(url, dev->url, sizeof(url));
        if(dev->submittype==http_get)
            fz_strlcat(url, data, sizeof(url)); 
        else// http_post
            postdata = data; 

        nh_do_request(helper, url, postdata);
    }
    fz_catch(ctx) {
        fz_rethrow(ctx);
    }
    return nh_get_http_code(helper);
}

int nh_login(http_helper *helper, nh_login_device *dev, ntko_server_info *svrinfo) 
{
    fz_context *ctx = nh_get_context(helper);
    if(!dev || !svrinfo || !dev->login)
        fz_throw(ctx, FZ_ERROR_GENERIC, "invalid paramater");

    fz_try(ctx) {
        dev->login(helper, dev, svrinfo);
    }
    fz_catch(ctx)
        fz_rethrow(ctx);

    return nh_get_http_code(helper); 
}

nh_login_device *nh_new_login_device_psw(fz_context *ctx, char* url, char *username, char *password) 
{
    nh_login_device_psw *psw_dev = null;
    fz_try(ctx) {
        nh_login_device *super = null;
        psw_dev = fz_malloc_struct(ctx, nh_login_device_psw);
        super = &psw_dev->super;
        super->ref = 1;
        super->submittype = http_post;
        super->url = fz_strdup(ctx, url);
        super->free = psw_login_device_free;
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
