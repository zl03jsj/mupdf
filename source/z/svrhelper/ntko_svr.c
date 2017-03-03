/*
 * =====================================================================================
 *
 *       Filename:  ntko_svr.c
 *    Description:  
 *        Version:  1.0
 *        Created:  2017/01/25 12时28分28秒
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  YOUR NAME (), 
 *   Organization:  
 * =====================================================================================
 */
#include <curl/curl.h>
#include <curl/easy.h> 
#include "mupdf/z/ntko_svr.h"
#include "mupdf/z/z_math.h"
#include <stdlib.h>

ntko_request nr_get_rootserver = {"GetRootServer", nrt_get_root_serverinfo,http_get};
ntko_request nr_login = {"login", nrt_login,http_post};
ntko_request nr_get_signlist = {"listsigns", nrt_get_signlist,http_get};
ntko_request nr_get_ekey_useable_state = {"checkekey", nrt_get_ekey_useable_state,http_get};
ntko_request nr_get_rights = {"getrights", nrt_check_right_handsign,http_get};
ntko_request nr_do_sign_log = {"dologsign", nrt_do_sign_log,http_get};
// ntko_request nr_get_binary_file = {"", nrt_get_binary_file,http_get};
ntko_request nr_logout = {"logoff", nrt_logout,http_get};
ntko_request nr_get_serverinfo = {"getserverinfo",nrt_test_connet_server,http_get};
ntko_request nr_get_server_time = {"getservertime", nrt_get_server_time,http_get};
ntko_request nr_check_sign = {"checksigns", nrt_check_sign,http_get};
ntko_request nr_get_server_changeinfo = {"getchkservers", nrt_get_server_changeinfo,http_get};
// ntko_request nr_down_loadfile = {null, nrt_download_file, http_get};
// ntko_request nr_get_right_of_ekeyandsign = {nrt_get_right_of_ekeyandsign,http_get};
// ntko_request nr_get_server_comments = {nrt_get_server_comments,http_get};
// ntko_request nr_set_server_comments = {nrt_set_server_comments,http_get};
// ntko_request nr_keep_session = {nrt_keep_session,http_get};

static const char *nxml_err_fmt_need_tag = "need xml tag:%s";
static const char *nxml_err_fmt_empty_att = "xml attribute is empty:%s";
// root item
static const char* nxml_NtkoSecsignResponse = "NtkoSecsignResponse";
static const char* nxml_ResponseStatus = "ResponseStatus";
static const char* nxml_ServerVersion = "ServerVersion";
static const char* nxml_Status = "Status"; 
static const char* nxml_ResponseInfo = "ResponseInfo";
static const char* nxml_RootServerInfo = "RootServerInfo";
static const char* nxml_ThisServerInfo = "ThisServerInfo";
static const char* nxml_ServerInfo = "Serverinfo";
static const char* nxml_ServerTime = "ServerTime";

static const char* nxml_att_version = "version";
static const char* nxml_att_errCode = "errCode";
static const char* nxml_att_failReason = "failReason";
static const char* nxml_att_RserverAppURL = "RseverAppUrl";
static const char* nxml_att_serverId = "serverId";
static const char* nxml_att_serverName = "serverName";
static const char* nxml_att_timeStamp = "timeStamp";
static const char* nxml_att_licenseUserName = "licenseUserName";
static const char* nxml_att_licenseCount = "licenseCount";

static
bool is_string_empty(const char *s)
{
    int c = !s ? 0: strlen(s);
    if(!c) return true;

    while(c && *s==' ') {c--; s++;}
    if(!c) return true;

    while(c && s[c-1]==' ') c--; 
    return c == 0 ? true : false;
}

static
fz_xml *nxml_find_down(fz_context *ctx, fz_xml *xml, const char *name)
{
    fz_xml *subxml = fz_xml_find_down(xml, name);
    if(!subxml)
        fz_throw(ctx, FZ_ERROR_XML, nxml_err_fmt_need_tag, name);
    return subxml;
}

static
const char *nxml_att(fz_context *ctx, fz_xml *xml, const char *att, bool is_throw_null_error)
{
    char *v = fz_xml_att(xml, att);
    if( is_string_empty(v) ) {
        if(is_throw_null_error) {
            fz_throw(ctx, FZ_ERROR_XML, nxml_err_fmt_empty_att, att);
        }
        else {
            fz_warn(ctx, nxml_err_fmt_empty_att, att);
        }
    }
    return v;
}

static
char *nxml_att_strdup(fz_context *ctx, fz_xml *xml, const char *att, bool is_throw_null_error)
{
    char *v = null;
    fz_try(ctx)
        v = fz_strdup(ctx, nxml_att(ctx, xml, att, is_throw_null_error));
    fz_catch(ctx)
        fz_rethrow(ctx);
    return v;
}

static 
bool nxml_att_bool(fz_context *ctx, fz_xml *xml, const char* att, bool def)
{
    char *value = fz_xml_att(xml, att);
    if(!value) {
        fz_warn(ctx, "null xml attribute:%s, return default value.", att); 
        return def;
    } 
    if(0==memcmp(value, "true", 4)) def = true;
    else if(0==memcmp(value, "false", 5)) def = false;
    else
        fz_warn(ctx, "failed to convert xml attribute to bool, att:%s, value:%s, use default value", att, value);
    return def;
}

static 
int nxml_att_int(fz_context *ctx, fz_xml *xml, const char *att, int def)
{
    char *value = fz_xml_att(xml, att);
    if(!value) {
        fz_warn(ctx, "null xml attribute:%s, return default value.", att); 
        return def;
    }

    if(value[0]<48 || value[0]>57)
        fz_warn(ctx, "failed to convert xml attribute to integer, attribute:%s, value:%s", att, value);
    else {
        def = atoi(value); 
    }
    return def; 
}

static
bool nxml_parse_user_right(fz_context *ctx, fz_xml *xml, ntko_user_rights *rights)
{
    if(!fz_xml_is_tag(xml, "ResponseInfo"))
        fz_throw(ctx, FZ_ERROR_GENERIC, "invalid xml node"); 

    fz_xml *xml_svrinfo, *xml_rights;
    xml_svrinfo = fz_xml_find_down(xml, "ThisServerInfo");
    xml_rights = fz_xml_find_down(xml, "UserRights");

    if(!xml_rights) 
        return false;

    rights->permit_handsign = nxml_att_bool(ctx, xml_rights, "canDoHandSign", false);
    rights->permit_barCode = nxml_att_bool(ctx, xml_rights, "canDoBarcode", false);
    rights->permit_keyboradSign = nxml_att_bool(ctx, xml_rights, "canDoKeyBoardComment", false);
    rights->permit_normalSign = nxml_att_bool(ctx, xml_rights, "canDoSign", false);
    rights->permit_ekeySign = nxml_att_bool(ctx, xml_rights, "canDoEkeySign", false);

    // sign server have no options, 
    // use key sign settings for default
    rights->permit_doubleCrossPage_sign = rights->permit_ekeySign;
    rights->permit_multiCrossPage_sign = rights->permit_ekeySign;
    rights->permit_multiPage_sign = rights->permit_ekeySign;
    rights->multiPage_sign_need_ekey = rights->permit_ekeySign;
    rights->multiCrossPage_sign_need_ekey= rights->permit_ekeySign; 
    return true;
}

static 
bool nxml_parse_sign_options(fz_context *ctx, fz_xml *xml, ntko_sign_options *option)
{
    if(!ctx || !xml || !option) 
        return false;

    if(!fz_xml_is_tag(xml, "ResponseInfo"))
        fz_throw(ctx, FZ_ERROR_GENERIC, "invalid xml node"); 

    fz_xml *subxml = null;
    fz_try(ctx){
        subxml = nxml_find_down(ctx, xml, "SignOptions");
        option->check_doc_modification = nxml_att_bool(ctx, subxml, "isCheckDocChange", true); 
        option->add_comment = nxml_att_bool(ctx, subxml, "isAddSignComment", false); 
        option->use_certificate = nxml_att_bool(ctx, subxml, "isUseCertificate", false); 
        option->printmode = nxml_att_bool(ctx, subxml, "PrintMode", 0); 
        option->show_option_ui = nxml_att_bool(ctx, subxml, "isShowUI", false); 
        option->lock_sign = nxml_att_bool(ctx, subxml, "isLocked", false); 
        option->saved = nxml_att_bool(ctx, subxml, "isSaveed", true); 
        option->check_font_change = nxml_att_bool(ctx, subxml, "isCheckFont", false); 
        option->sign_add_hand_draw = nxml_att_bool(ctx, subxml, "isAddSignHand", false); 
        option->sign_add_date = nxml_att_bool(ctx, subxml, "isAddSignDate", false); 
        option->csp_issure_name = nxml_att_strdup(ctx, subxml, "CSPIssuerName", false); 
    }
    fz_catch(ctx){
        fz_rethrow(ctx);
    }
    return true;
}

static 
z_list *nxml_parse_espinfo_list(fz_context *ctx, fz_xml *xml)
{
    z_list *l = NULL;
    fz_try(ctx) {
        fz_xml *subxml = fz_xml_find_down(xml, "NtkoUserSign");
        ntko_server_espinfo *info = NULL;
        while(subxml) {
            if(!l) l = ntko_espinfo_list_new(ctx);
            info = z_list_append_new(ctx, l); 
            info->signname = nxml_att_strdup(ctx, subxml, "signName", false);
            info->signuser = nxml_att_strdup(ctx, subxml, "signUser", false);
            info->filename = nxml_att_strdup(ctx, subxml, "signURL", false); 
            subxml = fz_xml_find_next(subxml, "NtkoUserSign"); 
        } 
    }
    fz_catch(ctx)
        fz_rethrow(ctx);
    return l;
}

//static
//void ntko_init_response_status(fz_context *ctx, ntko_http_response_status *status)
//{
//    status->code = -1;
//    if(status->fail_reason) {
//        fz_free(ctx, status->fail_reason);
//        status->fail_reason = null;
//    } 
//    status->user_param = ctx;
//}

// no matter returned ture or false,
// parse responsed xml complete, depend on response status code, 
// if status code is 0, return ture, else return false
// if parse failed, throw error
static
bool nxml_parse_response_status(fz_context *ctx, fz_xml *xml,  ntko_server_info *svrinfo,
        ntko_http_response_status *status) 
{
    if(!fz_xml_is_tag(xml,  "ResponseStatus"))
        fz_throw(ctx, FZ_ERROR_XML, "not ResponseStatus xml tag"); 

    char *tmp = null;
    fz_xml *xml_ver, *xml_status;
    xml_ver = fz_xml_find_down(xml, "ServerVersion");
    xml_status = fz_xml_find_down(xml, "Status"); 

    if(!xml_ver || !xml_status)
        fz_throw(ctx, FZ_ERROR_XML, "cann't find ServerVersion or Status xml tag"); 

    tmp = fz_xml_att(xml_ver, "version");
    if(!tmp)
        fz_throw(ctx, FZ_ERROR_XML, "xml tag version value empty"); 

    if(svrinfo->version) fz_free(ctx, svrinfo->version);
    svrinfo->version = fz_strdup(ctx, tmp); 

    status->code = nxml_att_int(ctx, xml_status, "errCode", -1); 

    bool isok = false;
    if(status->code!=0) {
        if(status->fail_reason) 
            fz_free(ctx, status->fail_reason);
        status->fail_reason = nxml_att_strdup(ctx, xml_status, "failReason", false);
    }
    else isok = true;

    return isok;
}

static
char *nxml_parse_server_time(fz_context *ctx, fz_xml *xml) 
{
    // it was make sure, xml tag name is checked before
    // invoke this static method,no need to check again
    if(!fz_xml_is_tag(xml, nxml_ResponseInfo))
        fz_throw(ctx, FZ_ERROR_GENERIC, nxml_err_fmt_need_tag, nxml_ResponseInfo); 

    char *strtime = null;
    fz_try(ctx){
        fz_xml *subxml = nxml_find_down(ctx, xml, nxml_ServerTime);
        strtime = nxml_att_strdup(ctx, subxml, nxml_att_timeStamp, true);
    }
    fz_catch(ctx) 
        fz_rethrow(ctx);

    return strtime; 
}

static
void ntko_set_root_server_info(fz_context *ctx, ntko_server_info *svrinfo, char *url)
{
    fz_try(ctx) {
        if(url && 0!=strlen(url) ) {
            if(svrinfo->rooturl) fz_free(ctx, svrinfo->rooturl);
            svrinfo->rooturl = fz_strdup(ctx, url);
        }
        else if(svrinfo->settingurl && 0!=strlen(svrinfo->settingurl)) {
            if(svrinfo->rooturl) fz_free(ctx,  svrinfo->rooturl);
            svrinfo->rooturl = fz_strdup(ctx, svrinfo->settingurl); 
        }
        else if(svrinfo->rooturl && 0!=strlen(svrinfo->rooturl)) {
            if(svrinfo->settingurl) fz_free(ctx, svrinfo->settingurl);
            svrinfo->settingurl = fz_strdup(ctx, svrinfo->rooturl); 
        }
        else 
            fz_throw(ctx, FZ_ERROR_GENERIC, "set sign server url failed"); 
    }
    fz_catch(ctx)
        fz_rethrow(ctx);
}

static 
bool nxml_parse_rootsvrinfo(fz_context *ctx, fz_xml*xml, ntko_server_info *svrinfo) 
{
    // it was make sure, xml tag name is checked before
    // invoke this static method,no need to check again
    if(!fz_xml_is_tag(xml, nxml_ResponseInfo))
        fz_throw(ctx, FZ_ERROR_GENERIC, nxml_err_fmt_need_tag, nxml_ResponseInfo); 
     
    fz_xml *subxml = null; 
    fz_try(ctx) {
        subxml = nxml_find_down(ctx, xml, nxml_RootServerInfo); 
        ntko_set_root_server_info(ctx, svrinfo, fz_xml_att(subxml, nxml_att_RserverAppURL));

        subxml = fz_xml_find_down(xml, nxml_ThisServerInfo); 
        if(subxml) {
            svrinfo->serverid = nxml_att_int(ctx, subxml, nxml_att_serverId, 0);
            svrinfo->servername = nxml_att_strdup(ctx, subxml, nxml_att_serverName, false);
        }
    }
    fz_catch(ctx) 
        fz_rethrow(ctx); 

    return true; 
}

static
bool nxml_parse_serverinfo(fz_context *ctx, fz_xml *xml, ntko_server_info *svrinfo)
{
    if(!fz_xml_is_tag(xml, nxml_ResponseInfo))
        fz_throw(ctx, FZ_ERROR_GENERIC, nxml_err_fmt_need_tag, nxml_ResponseInfo); 
     
    fz_xml *subxml = null; 
    fz_try(ctx) {
        subxml = nxml_find_down(ctx, xml, nxml_ServerInfo);

        if(svrinfo->version) fz_free(ctx, svrinfo->version);
        svrinfo->version = nxml_att_strdup(ctx, subxml, nxml_att_version, false);

        if(svrinfo->servername) fz_free(ctx, svrinfo->servername);
        svrinfo->servername = nxml_att_strdup(ctx, subxml, nxml_att_serverName, false);

        if(svrinfo->lic_username) fz_free(ctx, svrinfo->lic_username);
        svrinfo->lic_username = nxml_att_strdup(ctx, subxml, nxml_att_licenseUserName, false);

        svrinfo->lic_count = nxml_att_int(ctx, subxml, nxml_att_licenseCount, 0); 
    }
    fz_catch(ctx) 
        fz_rethrow(ctx); 

    return true; 

}

static
fz_xml* nxml_getl(fz_context *ctx, fz_xml *xml, char*first, ...) 
{
    char *tag = first;
	va_list valist;
	va_start(valist, first); 
    while(tag && xml) {
        xml = fz_xml_find_down(xml, tag);
        tag = va_arg(valist, char *);
    }
	va_end(valist); 
	return xml;
}

static 
bool ntko_is_http_inited = false;

void ntko_http_init()
{
    if(!ntko_is_http_inited)
        curl_global_init(CURL_GLOBAL_ALL);
    ntko_is_http_inited = true; 
}

static 
size_t http_response_fun(void* rspdata, size_t size, size_t nmemb, void* userdata)
{
    ntko_http_response_status *status = userdata;
    fz_context *ctx = status->user_param;
    size_t totalsize = size*nmemb;
    fz_try(ctx) {
        if(!status->data)
            status->data = fz_new_buffer(ctx, totalsize);
        fz_write_buffer(ctx, status->data, rspdata, totalsize);
    }
    fz_catch(ctx) {
        if(status->data) fz_drop_buffer(ctx, status->data);
        status->data = null;
        status->code = -1;
        fz_warn(ctx, "%s", fz_caught_message(ctx));
    }
    return totalsize; 
}

static 
size_t http_response_header_fun(void* header, size_t size, size_t nmemb, void *userdata)
{
    const char *response_header = "Set-Cookie: JSESSIONID="; 
    const char *request_header = "Cookie: JSESSIONID="; 

    ntko_http_response_status *status = userdata;
    char *sessionid = status->sessionid;
    int cap = sizeof(status->sessionid);

    int tmp = strlen(response_header);
    if(!memcmp(header, response_header, tmp)) {
        memset(sessionid, 0, cap);

        tmp = strlen(request_header);
        memcpy(sessionid, request_header, tmp);
        sessionid += tmp;

        header += strlen(response_header);
        tmp = 0;

        while( ((char*)header)[tmp]!=';' ) {
            sessionid[tmp] = ((char*)header)[tmp];
            tmp++;
        }
    } 
    return size*nmemb;
}

static
int ntko_make_url(char *url, int urlsize, char*base, char*sub, char *fields) 
{
    if(!url || !urlsize || !base) return 0;
    memset(url, 0, urlsize);

    int basesize = strlen(base);
    if(!basesize) return 0;

    int subsize = sub==null ? 0 :strlen(sub);
    int fieldssize = fields==null? 0 : strlen(fields);

    int size = basesize + subsize + fieldssize;

    if(subsize) {
        if(base[basesize-1]!='/') size++;
        if(fieldssize && sub[subsize-1]=='/') size--;
    }
    else
        if(base[basesize-1]=='/') size--;

    if(urlsize < (size+1)) return 0;

    memcpy(url, base, basesize); 
    url += basesize;

    if(subsize) {
        if(*(url-1)!='/') *url++ = '/';
        memcpy(url, sub, subsize);
        url += subsize;
    }

    if(fieldssize) {
        if(*(url-1)=='/') *(url-1) = '?';
        else *url++ = '?';
        memcpy(url, fields, fieldssize);
    } 
    *(++url) = '\0';

    return size; 
}

// if command failed, throw a error, http_helper->code is CURLcode, contain
// faild reason
// or command success
static
void ntko_do_http_command(fz_context *ctx, ntko_request *command,
         char *baseurl, char *fields,  ntko_http_response_status *status)
{
    char url[512];
    char *submit = null;

    CURL *httphandle = null; 
    if(fields && command->submit==http_post) {
        submit = fields;
        fields = null;
    }
    int size = ntko_make_url(url, sizeof(url), baseurl, command->name, fields); 
    if(!size)
        fz_throw(ctx, FZ_ERROR_GENERIC, "make http request url faild"); 

    status->user_param = ctx;

    ntko_http_init();
    httphandle = curl_easy_init();
    curl_easy_setopt(httphandle, CURLOPT_URL, url);
    if(submit) 
        curl_easy_setopt(httphandle, CURLOPT_POSTFIELDS, submit);

    curl_easy_setopt(httphandle, CURLOPT_HEADERFUNCTION, http_response_header_fun);
    curl_easy_setopt(httphandle, CURLOPT_HEADERDATA, status);
    curl_easy_setopt(httphandle, CURLOPT_WRITEFUNCTION, http_response_fun);
    curl_easy_setopt(httphandle, CURLOPT_WRITEDATA, status);
    
    size = strlen(status->sessionid);
    struct curl_slist *httphead = null;
    if(size) { 
        httphead = curl_slist_append(httphead, status->sessionid);
        curl_easy_setopt(httphandle, CURLOPT_HTTPHEADER, httphead); 
    }

    if(status->data)
        fz_buffer_reset(ctx, status->data);
    CURLcode code = curl_easy_perform(httphandle);

    if(httphead) {
        curl_slist_free_all(httphead);
        httphead = null;
    } 

    if(code!=CURLE_OK) {
        if(status->data) {
            fz_drop_buffer(ctx, status->data);
            status->data = null;
        }
        status->code = -1;

        fz_throw(ctx, FZ_ERROR_HTTP_REQUEST, 
                "http request(%s,%d) faild, message:%s", 
                command->name, command->request,
                curl_easy_strerror(status->code)); 
    }
}

static
fz_xml* ntko_xml_parse(fz_context *ctx, unsigned char *xmldata, int size)
{
    fz_xml *xml = null;
    fz_try(ctx) {
        xml = fz_parse_xml(ctx, xmldata, size, true);
        if(!xml) 
            fz_throw(ctx, FZ_ERROR_XML, "can't parse xml data");
        if(!fz_xml_is_tag(xml, "NtkoSecsignResponse"))
            fz_throw(ctx, FZ_ERROR_XML, "xml is not NtkoSecsignResponse"); 
    }
    fz_catch(ctx) {
        if(xml) fz_drop_xml(ctx, xml);
        xml = null;
        fz_rethrow(ctx);
    }
    return xml;
}

static 
bool ntko_http_get_root_svrinfo(fz_context *ctx, ntko_server_info *svrinfo, ntko_http_response_status *status)
{
    bool isok = false;

    fz_xml *xml = null;

    fz_try(ctx) {
        unsigned char *xmldata = null;
        int size;
        fz_xml *xml_rsp, *xml_rspinfo;
        ntko_do_http_command(ctx, &nr_get_rootserver, svrinfo->settingurl, null, status);
        size = fz_buffer_get_data(ctx, status->data, &xmldata);
        xml = ntko_xml_parse(ctx, xmldata, size);
        
        xml_rsp = fz_xml_find_down(xml, "ResponseStatus"); 
        xml_rspinfo = fz_xml_find_down(xml, "ResponseInfo");

        isok = nxml_parse_response_status(ctx, xml_rsp, svrinfo, status); 
        if(isok)
            isok = nxml_parse_rootsvrinfo(ctx, xml_rspinfo, svrinfo);
    }
    fz_always(ctx) {
        if(xml) fz_drop_xml(ctx, xml);
    }
    fz_catch(ctx) {
        isok = false;
        fz_rethrow(ctx);
    }
    return isok; 
}

bool ntko_http_login(fz_context *ctx, char *username, char *password,
        ntko_http_response_status *status, ntko_server_info *svrinfo,
        ntko_user_rights *rights) 
{
    bool isok = false;
    fz_xml *xml = null; 

    const char *fmt = "username=%s&password=%s&clientver=%s&lictype=%d";
    char fields[512];
    int size = 0;
    unsigned char *xmldata = null; 
    memset(fields, 0, sizeof fields);

    fz_try(ctx) {
        isok = ntko_http_get_root_svrinfo(ctx, svrinfo, status);
        if(isok) {
            size = snprintf(fields, sizeof fields,
                    fmt, username, password, "1.0", lic_all); 

            ntko_do_http_command(ctx, &nr_login, svrinfo->rooturl, fields, status); 
            size = fz_buffer_get_data(ctx, status->data, &xmldata);

            xml = ntko_xml_parse(ctx, xmldata, size); 

            fz_xml *subxml = null;
            subxml = nxml_find_down(ctx, xml, "ResponseStatus");
            isok = nxml_parse_response_status(ctx, subxml, svrinfo, status); 

            if(isok) {
                subxml = nxml_find_down(ctx, xml, "ResponseInfo"); 
                isok = nxml_parse_user_right(ctx, subxml, rights);
            }
        }
    } 
    fz_always(ctx) {
        if(xml) fz_drop_xml(ctx, xml);
    }
    fz_catch(ctx) {
        isok = false;
        fz_rethrow(ctx); 
    }
    return isok;
} 

bool ntko_http_logoff(fz_context *ctx, ntko_http_response_status *status, ntko_server_info *svrinfo)
{
    fz_try(ctx)
        ntko_do_http_command(ctx, &nr_logout, svrinfo->rooturl, null, status);
    fz_catch(ctx)
        fz_rethrow(ctx);

    return true; 
}

char *ntko_http_get_svrtime(fz_context *ctx, ntko_server_info *svrinfo,
        ntko_http_response_status *status)
{
    fz_xml *xml = null;
    char *time = null;
    fz_try(ctx){
        fz_xml *subxml = null;
        unsigned char *xmldata; 
        int size;
        bool isok = false;

        ntko_do_http_command(ctx, &nr_get_server_time, svrinfo->rooturl, null, status);
        size = fz_buffer_get_data(ctx, status->data, &xmldata);

        xml = ntko_xml_parse(ctx, xmldata, size);

        subxml = nxml_find_down(ctx, xml, nxml_ResponseStatus);
        isok = nxml_parse_response_status(ctx, subxml, svrinfo, status); 
        if(isok) {
            subxml = nxml_find_down(ctx, xml, nxml_ResponseInfo);
            time = nxml_parse_server_time(ctx, subxml); 
        } 
    }
    fz_always(ctx) {
        if(xml) fz_drop_xml(ctx, xml);
    }
    fz_catch(ctx){
        fz_rethrow(ctx);
    }
    return time;
}

// used to test server connection
bool ntko_http_get_svrinfo(fz_context *ctx, ntko_server_info *svrinfo,
        ntko_http_response_status *status)
{
    fz_xml *xml = null;
    bool isok = false; 
    fz_try(ctx){
        fz_xml *subxml = null;
        unsigned char *data; 
        int size;

        ntko_do_http_command(ctx, &nr_get_serverinfo, svrinfo->rooturl, null, status);

        size = fz_buffer_get_data(ctx, status->data, &data);

        xml = ntko_xml_parse(ctx, data, size); 

        subxml = nxml_find_down(ctx, xml, nxml_ResponseStatus);
        isok = nxml_parse_response_status(ctx, subxml, svrinfo, status); 
        if(isok) {
            subxml = nxml_find_down(ctx, xml, nxml_ResponseInfo);
            isok = nxml_parse_serverinfo(ctx, subxml, svrinfo); 
        } 
    }
    fz_always(ctx) {
        if(xml) fz_drop_xml(ctx, xml);
    }
    fz_catch(ctx){
        fz_rethrow(ctx);
    }
    return isok; 
}

z_list *ntko_http_get_esplist(fz_context *ctx, ntko_server_info *svrinfo,
        ntko_user_rights *rights, ntko_sign_options *options, ntko_http_response_status *status)
{
    fz_xml *xml = null;
    z_list *l = null;
    fz_try(ctx){
        fz_xml *subxml = null;
        unsigned char *data; 
        int size;

        ntko_do_http_command(ctx, &nr_get_signlist, svrinfo->rooturl, null, status);
        size = fz_buffer_get_data(ctx, status->data, &data);

        xml = ntko_xml_parse(ctx, data, size); 

        subxml = nxml_find_down(ctx, xml, nxml_ResponseStatus);
        bool isok = nxml_parse_response_status(ctx, subxml, svrinfo, status); 
        if(isok) {
            subxml = nxml_find_down(ctx, xml, nxml_ResponseInfo);
            isok = nxml_parse_user_right(ctx, subxml, rights); 
            if(isok)
                isok = nxml_parse_sign_options(ctx, subxml, options); 
            if(isok)
                l = nxml_parse_espinfo_list(ctx, subxml);
        } 
        else
            fz_warn(ctx, "request faild:%s", status->fail_reason);
    }
    fz_always(ctx) {
        if(xml) fz_drop_xml(ctx, xml);
    }
    fz_catch(ctx){
        fz_rethrow(ctx);
    }
    return l; 
}

bool ntko_http_get_user_rights(fz_context *ctx, ntko_server_info *svrinfo, ntko_user_rights *right, ntko_sign_options *options, ntko_http_response_status *status) 
{
    fz_xml *xml = null;

    bool isok = false;
    fz_try(ctx) {
        unsigned char *xmldata = null;
        int size = 0;
        ntko_do_http_command(ctx, &nr_get_rights, svrinfo->rooturl, null, status);
        size = fz_buffer_get_data(ctx, status->data, &xmldata);

        xml = ntko_xml_parse(ctx, xmldata, size);
        fz_xml *subxml = nxml_find_down(ctx, xml, nxml_ResponseStatus);

        isok = nxml_parse_response_status(ctx, subxml, svrinfo, status);
        if(isok) {
            subxml = nxml_find_down(ctx, xml, nxml_ResponseInfo);
            // TODO: parse this server info
            isok = nxml_parse_user_right(ctx, subxml, right);
            if(isok)
                isok = nxml_parse_sign_options(ctx, subxml, options); 
        } 
    }
    fz_always(ctx) {
        if(xml) fz_drop_xml(ctx, xml);
    }
    fz_catch(ctx)
        fz_rethrow(ctx);
    return isok;
}

bool ntko_http_dosign_log(fz_context *ctx, ntko_server_info *svrinfo, char *log,
       ntko_user_rights *right, ntko_http_response_status *status) 
{
    fz_xml *xml = null;

    bool isok = false;
    fz_try(ctx) {
        unsigned char *xmldata = null;
        int size = 0;
        ntko_do_http_command(ctx, &nr_get_rights, svrinfo->settingurl, log, status);
        size = fz_buffer_get_data(ctx, status->data, &xmldata);

        xml = ntko_xml_parse(ctx, xmldata, size);
        fz_xml *subxml = nxml_find_down(ctx, xml, nxml_ResponseStatus);
        isok = nxml_parse_response_status(ctx, subxml, svrinfo, status);
        if(isok) {
            subxml = nxml_find_down(ctx, xml, nxml_ResponseInfo);
            isok = nxml_parse_user_right(ctx, subxml, right);
        } 
    }
    fz_always(ctx) {
        if(xml) fz_drop_xml(ctx, xml);
    }
    fz_catch(ctx)
        fz_rethrow(ctx);

    return isok;
} 

bool ntko_http_download_esp(fz_context *ctx, ntko_server_info *info, ntko_server_espinfo *espinfo, ntko_http_response_status *status)
{
    if(espinfo->data) 
        return true;

    bool isok = false;
    fz_try(ctx) {
        ntko_request req = {espinfo->filename, nrt_get_binary_file, http_get}; 
        fz_buffer_reset(ctx, status->data);
        ntko_do_http_command(ctx, &req, info->rooturl, null, status);
        unsigned char *d = null;
        int size = fz_buffer_get_data(ctx, status->data, &d);

        if(espinfo->data)
            fz_buffer_reset(ctx, espinfo->data);
        else
            espinfo->data = fz_new_buffer(ctx, size); 

        fz_write_buffer(ctx, espinfo->data, d, size);

        isok = true;
    }
    fz_catch(ctx){
        fz_rethrow(ctx);
    }
    return isok; 
}

static 
z_list *nxml_parse_sign_check_server(fz_context *ctx, fz_xml *xml) 
{
    z_list *l = null;
    fz_try(ctx) {
        fz_xml *subxml = nxml_find_down(ctx, xml, "CheckSignsServerApp");
        ntko_check_sign_svrinfo *info = null;
        while(subxml) {
            if(!l) l = ntko_sign_check_svrinfo_list_new(ctx);
            info = z_list_append_new(ctx, l); 
            info->serverid = nxml_att_int(ctx, subxml, nxml_att_serverId, 0);
            info->sign_svr_name = nxml_att_strdup(ctx, subxml, nxml_att_serverName, false);
            info->server_app_url = nxml_att_strdup(ctx, subxml,nxml_att_RserverAppURL, true); 
            info->used_in_this_doc = false;
            info->postdata = null;

            subxml = fz_xml_find_next(subxml, "CheckSignServerApp");
        }
    }
    fz_catch(ctx)
        fz_rethrow(ctx);
    return l;
}

static 
z_list *ntko_http_get_check_servers(fz_context *ctx, ntko_server_info *svrinfo, ntko_http_response_status *status)
{
    fz_xml *xml = null;
    z_list *l = null;
    fz_try(ctx) {
        unsigned char *xmldata = null;
        int size = 0;
        fz_xml *subxml = null;
        bool isok = false;
        ntko_do_http_command(ctx, &nr_get_server_changeinfo, svrinfo->rooturl, null, status);
        size = fz_buffer_get_data(ctx, status->data, &xmldata);
        xml = ntko_xml_parse(ctx, xmldata, size);

        subxml = nxml_find_down(ctx, xml, nxml_ResponseStatus);
        isok = nxml_parse_response_status(ctx, subxml, svrinfo, status); 
        if(isok){
            subxml = nxml_find_down(ctx, xml, nxml_ResponseInfo);
            l = nxml_parse_sign_check_server(ctx, subxml); 
        }

    }
    fz_always(ctx){
        if(xml) fz_drop_xml(ctx, xml);
    }
    fz_catch(ctx){ 
        if(l) z_list_free(ctx, l);
        l = null;
    }
    return l;
}

static
void init_check_svr_postdata(fz_context *ctx, z_list *svrlist, z_list *signlist)
{
    int capacity = 256;
    int size = 0; 

    z_list_node *signnode = signlist->first;
    ntko_sign_info *signinfo = null;
    ntko_check_sign_svrinfo *svrinfo = null; 

    bool isok = true;
    while(signnode) {
        signinfo = signnode->data; 

        z_list_node *svrnode = svrlist->first; 
        while(svrnode) {
            svrinfo = svrnode->data; 

            if(svrinfo->serverid!=signinfo->serverid)
                continue;

            svrinfo->used_in_this_doc = true;

            int size1 = strlen("signguid=");
            int size2 = strlen(signinfo->sign_unid);

            if(!svrinfo->postdata) {
                capacity = fz_maxi(size1+size2+1, capacity);
                fz_try(ctx) {
                    svrinfo->postdata = fz_malloc(ctx, capacity);
                    isok = true;
                }
                fz_catch(ctx) {
                    fz_warn(ctx, "%s", fz_caught_message(ctx));
                    isok = false;
                }
            }
            else { 
                if( (size+size1+size2+2)>=capacity ) {
                    capacity += fz_maxi(size1+size2+2, 128);
                    fz_try(ctx) {
                        svrinfo->postdata = fz_realloc(ctx, svrinfo->postdata, capacity); 
                        isok = true;
                    }
                    fz_catch(ctx) {
                        fz_warn(ctx, "%s", fz_caught_message(ctx));
                        isok = false;
                    }
                }
                if(isok) svrinfo->postdata[size++] = '&';
            } 
            if(isok) { 
                memcpy(svrinfo->postdata + size, "signguid=", size1);
                size += size1;
                memcpy(svrinfo->postdata + size, signinfo->sign_unid, size2);
                size += size2; 
            }
        } 
    } 
}

static
bool nxml_parse_sign_check_status(fz_context *ctx, fz_xml *xml, z_list *svrsignlist)
{
    fz_xml *subxml = null;
    bool isok = true;
    fz_try(ctx) {
        // here use fz_xml_find_down
        // not throw error
        subxml = nxml_find_down(ctx, xml, "SignsCheckStatus");
        while(subxml) {
            ntko_server_sign_info *info = z_list_append_new(ctx, svrsignlist);
            info->sign_unid = nxml_att_strdup(ctx, subxml, "signUNID", true);
            info->found = nxml_att_bool(ctx, subxml, "found", false);
            if(!info->found)
                continue;
            info->signer = nxml_att_strdup(ctx, subxml, "signer", false);
            info->sign_name = nxml_att_strdup(ctx, subxml, "signName", false);
            info->sign_user = nxml_att_strdup(ctx, subxml, "signUser", false);
            info->sign_sn = nxml_att_strdup(ctx, subxml, "signSN", false);
            info->ekey_sn = nxml_att_strdup(ctx, subxml, "ekeySN", false);
            info->server_time = nxml_att_strdup(ctx, subxml, "serverTime", false);
            info->app_name = nxml_att_strdup(ctx, subxml, "appName", false);
            info->client_ip = nxml_att_strdup(ctx, subxml, "clinetIP", false);

            subxml = fz_xml_find_next(subxml, "SignsCheckStatus");
        }
    }
    fz_catch(ctx) {
        isok = false;
        fz_rethrow(ctx);
    }
    return isok;
}
        

bool ntko_http_check_sign_one_serverurl(fz_context *ctx, ntko_server_info *svrinfo, 
        ntko_http_response_status *status, ntko_check_sign_svrinfo *checksvrinfo,
        z_list *svrsignlist)
{
    fz_xml *xml = null;
    bool isok = false;
    fz_try(ctx) {
        fz_xml *subxml = null;
        unsigned char *xmldata = null;
        int size = 0;
        ntko_do_http_command(ctx, &nr_check_sign, svrinfo->rooturl, checksvrinfo->postdata, status);
        size = fz_buffer_get_data(ctx, status->data, &xmldata);

        xml = ntko_xml_parse(ctx, xmldata, size);
        subxml = nxml_find_down(ctx, xml, nxml_ResponseStatus);
        
        isok = nxml_parse_response_status(ctx, subxml, svrinfo, status);
        if(isok) {
            subxml = nxml_find_down(ctx, subxml, nxml_ResponseInfo);
            isok = nxml_parse_sign_check_status(ctx, subxml, svrsignlist); 
        } 
    }
    fz_always(ctx) {
        if(xml) fz_drop_xml(ctx, xml);
    }
    fz_catch(ctx) {
        fz_rethrow(ctx); 
    }
    return isok;
}

z_list *ntko_http_check_signs(fz_context *ctx, ntko_server_info *svrinfo, 
        ntko_http_response_status *status, z_list *signlist)
{
    z_list *svrlist = null; 
    z_list *svr_sign_list = null;
    fz_try(ctx) {
        svrlist = ntko_http_get_check_servers(ctx, svrinfo, status);
        init_check_svr_postdata(ctx, svrlist, signlist); 

        z_list_node *node = svrlist->first;
        ntko_check_sign_svrinfo *info = null;
        while(node) {
            info = node->data;
            if(!info)
                continue;

            if(!svr_sign_list)
                svr_sign_list = ntko_svr_signinfo_list_new(ctx);
            ntko_http_check_sign_one_serverurl(ctx, svrinfo, status, info, svr_sign_list); 
            node = node->n;
        }
    }
    fz_always(ctx)
        if(svrlist) z_list_free(ctx, svrlist); 
    fz_catch(ctx) {
        if(svr_sign_list) z_list_free(ctx, svr_sign_list);
        svr_sign_list = null; 
    }
    return svr_sign_list;
}

static 
void *espinfo_new(fz_context *ctx)
{
    ntko_server_espinfo *espinfo = NULL;
    fz_try(ctx) {
        espinfo = fz_malloc_struct(ctx, ntko_server_espinfo);
        espinfo->ref = 1;
    }
    fz_catch(ctx)
        fz_rethrow(ctx);
    return espinfo;
}

static
void espinfo_drop(fz_context *ctx, void *data)
{
    ntko_server_espinfo *info = (ntko_server_espinfo*)data;
    fz_try(ctx) {
        ntko_drop_server_espinfo(ctx, info);
    }
    fz_catch(ctx) {
        fz_rethrow(ctx);
    }
}

z_list *ntko_espinfo_list_new(fz_context *ctx) 
{
    z_list *l = NULL;
    fz_try(ctx)
        l = z_list_new(ctx, espinfo_new, espinfo_drop);
    fz_catch(ctx)
        fz_rethrow(ctx);
    return l; 
}

static
void *check_sign_svrinfo_new(fz_context *ctx)
{
    ntko_check_sign_svrinfo *info = null;
    fz_try(ctx)
        info = fz_malloc_struct(ctx, ntko_check_sign_svrinfo);
    fz_catch(ctx)
        fz_rethrow(ctx);
    return info;
}

static
void check_sign_svrinfo_drop(fz_context *ctx, void *data)
{
    ntko_check_sign_svrinfo *info = data;
    fz_try(ctx) {
        if(info->sign_svr_name) fz_free(ctx, info->sign_svr_name);
        if(info->server_app_url) fz_free(ctx, info->server_app_url);
        if(info->postdata) fz_free(ctx, info->postdata);
        fz_free(ctx, info);
    }
    fz_catch(ctx)
        fz_rethrow(ctx); 
}
 
z_list *ntko_sign_check_svrinfo_list_new(fz_context *ctx) 
{
    z_list *l = NULL;
    fz_try(ctx)
        l = z_list_new(ctx, check_sign_svrinfo_new, check_sign_svrinfo_drop);
    fz_catch(ctx)
        fz_rethrow(ctx);
    return l; 
}

static 
void *signinfo_new(fz_context *ctx)
{
    ntko_server_espinfo *espinfo = NULL;
    fz_try(ctx)
        espinfo = fz_malloc_struct(ctx, ntko_server_espinfo);
    fz_catch(ctx)
        fz_rethrow(ctx);
    return espinfo;
}

static
void signinfo_drop(fz_context *ctx, void *data)
{
    ntko_server_espinfo *info = (ntko_server_espinfo*)data;
    fz_try(ctx) {
        if(info->signname) fz_free(ctx,info->signname);
        if(info->signuser) fz_free(ctx,info->signuser);
        if(info->filename) fz_free(ctx,info->filename);
        if(info->data) fz_drop_buffer(ctx, info->data);
        fz_free(ctx, info); 
    }
    fz_catch(ctx)
        fz_rethrow(ctx);
}

z_list *ntko_signinfo_list_new(fz_context *ctx) 
{
    z_list *l = NULL;
    fz_try(ctx)
        l = z_list_new(ctx, signinfo_new, signinfo_drop);
    fz_catch(ctx)
        fz_rethrow(ctx);
    return l; 
}

static 
void *svr_signinfo_new(fz_context *ctx)
{
    ntko_server_sign_info *info = NULL;
    fz_try(ctx)
        info = fz_malloc_struct(ctx, ntko_server_sign_info);
    fz_catch(ctx)
        fz_rethrow(ctx);
    return info;
}

static
void svr_signinfo_drop(fz_context *ctx, void *data)
{
    ntko_server_sign_info *info = data;
    fz_try(ctx) {
        if(info->sign_unid) fz_free(ctx, info->sign_unid);
        if(info->signer) fz_free(ctx, info->signer);
        if(info->sign_name) fz_free(ctx, info->sign_name);
        if(info->sign_user) fz_free(ctx, info->sign_user);
        if(info->sign_sn) fz_free(ctx, info->sign_sn);
        if(info->ekey_sn) fz_free(ctx, info->ekey_sn);
        if(info->server_time) fz_free(ctx, info->server_time);
        if(info->app_name) fz_free(ctx, info->app_name);
        if(info->client_ip) fz_free(ctx, info->client_ip);
        fz_free(ctx, info);
    }
    fz_catch(ctx)
        fz_rethrow(ctx);
}

z_list *ntko_svr_signinfo_list_new(fz_context *ctx) 
{
    z_list *l = NULL;
    fz_try(ctx)
        l = z_list_new(ctx, svr_signinfo_new, svr_signinfo_drop);
    fz_catch(ctx)
        fz_rethrow(ctx);
    return l; 
}


void ntko_drop_server_espinfo(fz_context *ctx, ntko_server_espinfo *espinfo) {
    espinfo->ref--;
    if(espinfo->ref <=0 ) {
        if(espinfo->filename) fz_free(ctx, espinfo->filename);
        if(espinfo->signuser) fz_free(ctx, espinfo->signuser);
        if(espinfo->signname) fz_free(ctx, espinfo->signname);
        if(espinfo->data) fz_drop_buffer(ctx, espinfo->data);

        fz_free(ctx, espinfo);
    }
}
ntko_server_espinfo *ntko_keep_server_espinfo(fz_context *ctx, ntko_server_espinfo *espinfo) {
    espinfo->ref++;
    return espinfo;
}
