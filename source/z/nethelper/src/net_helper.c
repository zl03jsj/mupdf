/*
 * =====================================================================================
 *       Filename:  nethelper.c
 *    Description:  help to interactive with ntko sign server.
 *        Version:  1.0
 *        Created:  2017/01/24 15时42分06秒
 *       Revision:  none
 *       Compiler:  gcc *
 *         Author:  YOUR NAME (zl), 
 *   Organization:  ntko.com
 * =====================================================================================
 */
#include "nh_type.h"
#include "net_helper.h" 
#include <curl/curl.h>
#include <curl/easy.h> 

struct ntko_http_helper_s {
    fz_context *ctx;
    CURL *curl; 
}; 

struct curl_callback_data_s{
    fz_context *ctx;
    fz_buffer *buff;
};
typedef struct curl_callback_data_s curl_callback_data;

static bool is_global_inited = false;

static size_t nh_curl_performed(void* ptr, size_t size, size_t nmemb, void*data)
{
    curl_callback_data *pd = (curl_callback_data*)data;
    fz_context *ctx = pd->ctx;

    unsigned int dsize = 0;
    unsigned char *d = null;
    fz_try(ctx) 
    {
        dsize =  size * nmemb;
        d = fz_malloc(ctx, dsize);
        memcpy(d, ptr, dsize); 
        if(pd->buff) 
            fz_drop_buffer(ctx, pd->buff);
        pd->buff = fz_new_buffer_from_data(ctx, d, dsize);
    }
    fz_catch(ctx) {
        if(d) fz_free(ctx, d);
        dsize = 0;
        fz_warn(ctx, "something woring after performed url:\n%s", fz_caught_message(ctx)); 
    }
    return dsize; 
}

void nh_init() 
{
    // TODO: lock, to make thread safe
    if(!is_global_inited)
        curl_global_init(CURL_GLOBAL_DEFAULT);
    is_global_inited = true;
}

ntko_http_helper *nh_new(fz_context *ctx) 
{
    ntko_http_helper *hp = fz_malloc_struct(ctx, ntko_http_helper);
    hp->ctx = ctx;
    hp->curl = curl_easy_init();
}

fz_context* nh_get_context(ntko_http_helper *helper)
{
    return helper->ctx;
}

void* nh_get_curl(ntko_http_helper *helper) 
{
    return (void*)helper->curl;
}

http_data *nh_do_post(ntko_http_helper *helper, char* url, char *postdata) 
{
    // TODO: lock, to make thread safe;
    return null;
}

http_data *nh_do_get(ntko_http_helper *helper, char* url, char *getdata) 
{
    // TODO: lock, to make thread safe;
    return null;
}

void nh_free(ntko_http_helper *helper, bool freectx) 
{ 
    if(helper->curl)
        curl_easy_cleanup(helper->curl); 

    fz_context *ctx = helper->ctx;
    fz_free(ctx, helper);

    if(freectx && ctx) {
        fz_drop_context(ctx);
    } 
}


