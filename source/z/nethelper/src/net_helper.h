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

#ifndef http_helper_h__
#define http_helper_h__
#include "nh_type.h"
typedef struct ntko_server_info_s ntko_server_info;

typedef enum {
    http_post = 0,
    http_get 
} nh_submit_method;

typedef struct http_helper_s http_helper; 
typedef struct nh_login_device_s  nh_login_device;

#if 0
struct http_data_s {
   int status_code;
   fz_buffer *data;
};
typedef struct http_data_s http_data;
#endif

void nh_init(); 
http_helper *nh_new(fz_context *ctx);
void nh_free(http_helper *helper); 

fz_context* nh_get_context(http_helper *helper);
fz_buffer* nh_get_http_data(http_helper *helper);
int nh_get_http_code(http_helper *helper);

// submit request to url
// concat get fields to submit to the right side of url dirctly
// postdata, post fields
// if request was submit and get an response, return ture
bool nh_do_request(http_helper *helper, char* url, char *postdata); 

nh_login_device *nh_new_login_device_psw(fz_context *ctx, char *url, char *username, char *password);

void nh_drop_login_device(fz_context *ctx, nh_login_device *dev);

nh_login_device *nh_keep_login_device(fz_context *ctx, nh_login_device *dev);

// login
int nh_login(http_helper *helper, nh_login_device *dev, ntko_server_info *svrinfo);

// check http code, if ok return true, or return false
// describe , the description string of code, don't free it
bool nh_check_http_code(fz_context *ctx, int code, char **describe);

// check the http code in helper, if ok return true, or return false
// inner use nh_check_http_code
// return true, if the http request is submit, 
// and we get an http response, else return false
bool nh_check_status(http_helper *helper);

// return buffer form last request, and set to null, must be droped
// code is set to orignal state
fz_buffer *nh_reset_status(http_helper *helper, int *code);

// drop buffer from last request and set to null
// code is set to orignal state
void nh_clear_status(http_helper *helper);


#endif
