/*
 * =====================================================================================
 *       Filename:  ntkohelper.h
 *    Description:  
 *        Version:  1.0
 *        Created:  2017/01/24 17时48分44秒
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  YOUR NAME (), 
 *   Organization:  
 * =====================================================================================
 */

#include "nh_type.h"

typedef enum {
    post = 0,
    get 
} nh_submit_method;

typedef struct ntko_http_helper_s ntko_http_helper; 
typedef struct nh_login_device_s  nh_login_device;

struct http_data_s {
    int status_code;
    fz_buffer *data;
};
typedef struct http_data_s http_data;

void nh_init(); 
ntko_http_helper *nh_new(fz_context *ctx);
void nh_free(ntko_http_helper *helper, bool freectx); 
fz_context* nh_get_context(ntko_http_helper *helper);
// void* nh_get_curl(ntko_http_helper *helper);
http_data *nh_do_post(ntko_http_helper *helper, char* url, char *postdata);
http_data *nh_do_get (ntko_http_helper *helper, char* url, char *getdata);

nh_login_device *nh_new_login_device_psw(fz_context *ctx, char *url, char *username, char *password);
void nh_drop_login_device(fz_context *ctx, nh_login_device *dev);
nh_login_device *nh_keep_login_device(fz_context *ctx, nh_login_device *dev);
void nh_drop_login_device(fz_context *ctx, nh_login_device *dev);
int nh_login(ntko_http_helper *helper, nh_login_device *dev);

