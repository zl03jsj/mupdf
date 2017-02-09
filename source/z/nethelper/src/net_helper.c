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
#include "net_helper.h" 
#include <curl/curl.h>
#include <curl/easy.h> 

struct http_helper_s {
    fz_context *ctx;
    CURL *curl; 
    CURLcode status;
    fz_buffer *buf;
}; 

static bool is_global_inited = false; 

static size_t nh_curl_performed(void* ptr, size_t size, size_t nmemb, void* data)
{
    http_helper *helper = (http_helper*)data;
    fz_context *ctx = helper->ctx;
    unsigned int dsize = 0;
    unsigned char *d = null;
    fz_try(ctx) 
    {
        fz_buffer *buf;
        dsize =  size * nmemb;
        d = fz_malloc(ctx, dsize);
        memcpy(d, ptr, dsize); 

        buf = helper->buf;
        helper->buf = null;
        if(buf) fz_drop_buffer(ctx, buf);

        helper->buf= fz_new_buffer_from_data(ctx, d, dsize);
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

http_helper *nh_new(fz_context *ctx) 
{
    http_helper *hp = fz_malloc_struct(ctx, http_helper);
    hp->ctx = ctx;
    nh_init();
    hp->curl = curl_easy_init();
    hp->status = CURL_LAST;
    hp->buf = null;
    return hp;
}

void* nh_get_curl(http_helper *helper) {
    return (void*)helper->curl;
}

fz_context* nh_get_context(http_helper *helper) {
    return helper->ctx;
}

fz_buffer* nh_get_http_data(http_helper *helper) {
    return helper->buf;
}

int nh_get_http_code(http_helper *helper) {
    return helper->status;
}

#if 0
static
http_data *new_http_data(fz_context *ctx, CURLcode code, fz_buffer *buf) 
{
   http_data *hd = null;
   fz_try(ctx) {
       hd = fz_malloc_struct(ctx, http_data);
       hd->data = buf;
       hd->status_code = code;
   }
   fz_catch(ctx){
       fz_rethrow(ctx);
   }
   return hd;
}
#endif

bool nh_do_request(http_helper *helper, char* url, char *postdata) 
{
    // fz_context *ctx = helper->ctx;
    CURL *curl = helper->curl; 
    // TODO: lock, to make thread safe;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    if(postdata!=null)
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata); 
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, helper);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, nh_curl_performed); 
    // curl_easy_setopt(curl, CURLOPT_USERPWD, "SUREN:SUREN");
    /*
     * If you want to connect to a site who isn't using a certificate that is
     * signed by one of the certs in the CA bundle you have, you can skip the
     * verification of the server's certificate. This makes the connection
     * A LOT LESS SECURE.
     *
     * If you have a CA cert for the server stored someplace else than in the
     * default bundle, then the CURLOPT_CAPATH option might come handy for
     * you.
     */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    /*
     * If the site you're connecting to uses a different host name that what
     * they have mentioned in their server certificate's commonName (or
     * subjectAltName) fields, libcurl will refuse to connect. You can skip
     * this check, but this will make the connection less secure.
     */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    helper->status = curl_easy_perform(curl);

    return helper->status==CURLE_OK?true:false;
} 

void nh_free(http_helper *helper) 
{ 
    fz_context *ctx = helper->ctx;
    fz_try(ctx) 
    { 
        if(helper->curl)
            curl_easy_cleanup(helper->curl);
        if(helper->buf)
            fz_drop_buffer(ctx, helper->buf); 
        fz_free(ctx, helper);
    }
    fz_catch(ctx){
        fz_rethrow(ctx);
    }
}

bool nh_check_status(http_helper *helper) {
    fz_context *ctx = helper->ctx; 
    bool isok = false;
    fz_try(ctx) {
        isok = nh_check_http_code(ctx, helper->status, null);
    }
    fz_catch(ctx) {
        fz_rethrow(ctx);
    }
    return isok;
}

bool nh_check_http_code(fz_context *ctx, int code, char **describe) {
    bool isok = false;

    if(code==CURLE_OK) {
        isok = true; 
        if(describe) {
            *describe = "ok";
        }
    }
    else {
        isok = false;
        if(describe) {
            *describe = "http error";
        }
    }
    
    return isok; 
}

fz_buffer *nh_reset_status(http_helper *helper, int *code) 
{
    fz_buffer *buf = helper->buf;
    if(code) *code = helper->status;

    helper->status = CURL_LAST;
    helper->buf = null;

    return buf; 
}

void nh_clear_status(http_helper *helper) {
    fz_context *ctx = helper->ctx;
    fz_try(ctx) {
        if(helper->buf) fz_drop_buffer(ctx, helper->buf);
        helper->buf = null;
        helper->status = CURL_LAST;
    }
    fz_catch(ctx)
        fz_rethrow(ctx); 
}
