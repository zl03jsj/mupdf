/*
 * =====================================================================================
 *
 *       Filename:  ntko_helper.c
 *    Description:  
 *        Version:  1.0
 *        Created:  2017/01/25 12时28分28秒
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  YOUR NAME (), 
 *   Organization:  
 * =====================================================================================
 */
#include "ntko_helper.h"
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

#if 0
typedef struct ntko_http_field_s {
    char *key;
    char *data;
    int data_size;
}ntko_http_field;

struct ntko_http_fields_list_s {
  ntko_http_field *cur;  
  ntko_http_field *next;  
} ntko_httt_fields_list;
#endif

static
fz_xml *nxml_find_down(fz_context *ctx, fz_xml *xml, const char *name)
{
    fz_xml *subxml = fz_xml_find_down(xml, name);
    if(!subxml)
        fz_throw(ctx, FZ_ERROR_XML, nxml_err_fmt_need_tag, name);
    return subxml;
}

static
const char *nxml_att(fz_context *ctx, fz_xml *xml, const char *att)
{
    char *v = fz_xml_att(xml, att);
    if(!v)
        fz_throw(ctx, FZ_ERROR_XML, nxml_err_fmt_empty_att, att);
    return v;
}

static
char *nxml_att_strdup(fz_context *ctx, fz_xml *xml, const char *att)
{
    char *v = null;
    fz_try(ctx) {
        v = fz_strdup(ctx, nxml_att(ctx, xml, att)); 
    }
    fz_catch(ctx)
        fz_rethrow(ctx);
    return v;
}

static
char *nxml_att_strdup_nothrow(fz_context *ctx, fz_xml *xml, const char *att)
{
    char *v = null;
    fz_try(ctx) {
        const char *t = nxml_att(ctx, xml, att);
        if(t) v = fz_strdup(ctx, t);
    }
    fz_catch(ctx)
        fz_warn(ctx, "%s", fz_caught_message(ctx));
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

    if(0==memcmp(value, "true", 4))
        def = true;
    else if(0==memcpy(value, "false", 5))
        def = false;
    else
        fz_warn(ctx, "failed to convert xml attribute to bool, attribute:%s, value:%s, use default value", att, value);
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

    if(value[0]<48 &&value[0]>57)
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

    if(!xml_svrinfo || !xml_rights) 
        return false;
//        fz_throw(ctx, FZ_ERROR_GENERIC, "invalid xml node"); 

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
        option->csp_issure_name = nxml_att_strdup(ctx, subxml, "CSPIssuerName"); 
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
            info->signname = nxml_att_strdup_nothrow(ctx, subxml, "signName");
            info->signuser = nxml_att_strdup_nothrow(ctx, subxml, "signUser");
            info->filename = nxml_att_strdup_nothrow(ctx, subxml, "signURL"); 
            subxml = fz_xml_find_next(subxml, "NtkoUserSign"); 
        } 
    }
    fz_catch(ctx)
        fz_rethrow(ctx);
    return l;
}

static
void ntko_init_response_status(fz_context *ctx, ntko_http_response_status *status)
{
    if(status->failreason)
        fz_free(ctx, status->failreason);
    status->failreason = null;
    status->code = -1;
}

// no matter returned ture or false,
// parse responsed xml complete, depend on response status code, 
// if status code is 0, return ture, else return false
// if parse failed, throw error
static
bool nxml_parse_response_status(fz_context *ctx, fz_xml *xml,  ntko_server_info *svrinfo,
        ntko_http_response_status *status) 
{
    ntko_init_response_status(ctx, status);
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

    status->code = nxml_att_int(ctx, xml, "errCode", -1); 

    bool isok = false;
    if(status->code!=0) {
        char *reason = fz_xml_att(xml_status, "failReason"); 
        if(reason)
            status->failreason = fz_strdup(ctx, reason);
    }
    else
        isok = true;

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
        strtime = nxml_att_strdup(ctx, subxml, nxml_att_timeStamp);
    }
    fz_catch(ctx) 
        fz_rethrow(ctx);

    return strtime; 
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
        svrinfo->rooturl = nxml_att_strdup(ctx, subxml, nxml_att_RserverAppURL);

        subxml = fz_xml_find_down(xml, nxml_ThisServerInfo); 
        if(subxml) {
            svrinfo->serverid = nxml_att_int(ctx, subxml, nxml_att_serverId, 0);
            svrinfo->servername = nxml_att_strdup_nothrow(ctx, subxml, nxml_att_serverName);
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
        svrinfo->version = nxml_att_strdup(ctx, subxml, nxml_att_version);

        if(svrinfo->servername) fz_free(ctx, svrinfo->servername);
        svrinfo->servername = nxml_att_strdup(ctx, subxml, nxml_att_serverName);

        if(svrinfo->lic_username) fz_free(ctx, svrinfo->lic_username);
        svrinfo->lic_username = nxml_att_strdup(ctx, subxml, nxml_att_licenseUserName);

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

// if command failed, throw a error, http_helper->code is CURLcode, contain
// faild reason
// or command success
static
void ntko_do_http_command(http_helper *helper, ntko_request *command, char *baseurl, char *fields)
{
    fz_context *ctx = nh_get_context(helper);

    bool isok = false;
    char url[512];
    int size = 0;
    char *submit = null;
    memset(url, 0, sizeof url);

    size = strlen(baseurl);
    memcpy(url, baseurl, size); 

    int tmpsize = 0;
    if(command->name) 
    {
        strlen(command->name);
        if(tmpsize && url[size-1]!='/')
            url[size++] = '/';
        memcpy(url+size, command->name, tmpsize);
        size += tmpsize;
    }

    if(command->type==http_get && fields) 
    {
        tmpsize = strlen(fields);
        url[size++] = '?';
        memcpy(url+size, fields, tmpsize);
        size += tmpsize;
    }
    else  // http_post
        submit = fields;

    isok = nh_do_request(helper, url, submit); 
    if(!isok) 
        fz_throw(ctx, FZ_ERROR_HTTP_REQUEST, "http command faild:command(%s, %d)",
                command->name, command->type);
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
bool ntko_http_get_root_svrinfo(http_helper *helper, ntko_server_info *svrinfo, ntko_http_response_status *rps_status)
{
    bool isok = false;

    fz_context *ctx = nh_get_context(helper);
    fz_buffer *xmlbuff = null;
    fz_xml *xml = null;

    ntko_init_response_status(ctx, rps_status);

    fz_try(ctx) {
        unsigned char *xmldata = null;
        int size;
        fz_xml *xml_rsp, *xml_rspinfo;
        ntko_do_http_command(helper, &nr_get_rootserver, svrinfo->queryurl, null);
        xmlbuff = nh_reset_status(helper, null);

        if(!xmlbuff)
            fz_throw(ctx, FZ_ERROR_XML, "http command resposne data buffer empty"); 
        size = fz_buffer_get_data(ctx, xmlbuff, &xmldata);
        xml = ntko_xml_parse(ctx, xmldata, size);
        
        xml_rsp = fz_xml_find_down(xml, "ResponseStatus"); 
        xml_rspinfo = fz_xml_find_down(xml, "ResponseInfo");

        isok = nxml_parse_response_status(ctx, xml_rsp, svrinfo, rps_status); 
        if(isok)
            isok = nxml_parse_rootsvrinfo(ctx, xml_rspinfo, svrinfo);
    }
    fz_always(ctx) {
        if(xmlbuff) fz_drop_buffer(ctx, xmlbuff);
        if(xml) fz_drop_xml(ctx, xml);
    }
    fz_catch(ctx) {
        isok = false;
        fz_rethrow(ctx);
    }
    return isok; 
}

bool ntko_http_login(http_helper *helper, char *username, char *password,
        ntko_http_response_status *rsp_status, ntko_server_info *svrinfo,
        ntko_user_rights *rights) 
{
    bool isok = false;
    fz_context *ctx = nh_get_context(helper);
    fz_buffer *xmlbuff = null;
    fz_xml *xml = null; 

    const char *fmt = "username=%s&password=%sclientver=%s&lictype=%d";
    char fields[512];
    int size = 0;
    unsigned char *xmldata = null; 
    memset(fields, 0, sizeof fields);

    fz_try(ctx) {
        isok = ntko_http_get_root_svrinfo(helper, svrinfo, rsp_status);

        size = snprintf(fields, sizeof fields,
                fmt, username, password, "1.0", lic_all); 
        ntko_do_http_command(helper, &nr_login, svrinfo->rooturl, fields); 

        xmlbuff = nh_reset_status(helper, null);
        if(!xmlbuff)
            fz_throw(ctx, FZ_ERROR_XML, "xml data buffer is null"); 

        size = fz_buffer_get_data(ctx, xmlbuff, &xmldata);

        xml = ntko_xml_parse(ctx, xmldata, size); 

        fz_xml *rsp=null, *rspinfo =null;
        rsp = nxml_find_down(ctx, xml, "ResponseStatus");
        rspinfo = nxml_find_down(ctx, xml, "ResponseInfo"); 

        isok = nxml_parse_response_status(ctx, rsp, svrinfo, rsp_status);
        if(isok)
            isok = nxml_parse_user_right(ctx, rspinfo, rights);
    } 
    fz_always(ctx) {
        if(xmlbuff) fz_drop_buffer(ctx, xmlbuff);
        if(xml) fz_drop_xml(ctx, xml);
    }
    fz_catch(ctx) {
        isok = false;
        fz_rethrow(ctx); 
    }
    return isok;
} 

bool ntko_http_logoff(http_helper *helper, ntko_server_info *svrinfo)
{
    fz_context *ctx = nh_get_context(helper);
    fz_try(ctx)
        ntko_do_http_command(helper, &nr_logout, svrinfo->rooturl, null);
    fz_catch(ctx)
        fz_rethrow(ctx);

    return true; 
}

char *ntko_http_get_svrtime(http_helper *helper, ntko_server_info *svrinfo,
        ntko_http_response_status *status)
{
    fz_context *ctx = nh_get_context(helper);
    fz_buffer *buf = null;
    fz_xml *xml = null;
    char *time = null;
    fz_try(ctx){
        fz_xml *subxml = null;
        unsigned char *data; 
        int size;
        bool isok = false;

        ntko_init_response_status(ctx, status);

        ntko_do_http_command(helper, &nr_get_server_time, svrinfo->rooturl, null);
        buf = nh_reset_status(helper, null);
        size = fz_buffer_get_data(ctx, buf, &data);

        xml = ntko_xml_parse(ctx, data, size);

        subxml = nxml_find_down(ctx, xml, nxml_ResponseStatus);
        isok = nxml_parse_response_status(ctx, subxml, svrinfo, status); 
        if(isok) {
            subxml = nxml_find_down(ctx, xml, nxml_ResponseInfo);
            time = nxml_parse_server_time(ctx, subxml); 
        } 
    }
    fz_always(ctx) {
        if(buf) fz_drop_buffer(ctx, buf);
        if(xml) fz_drop_xml(ctx, xml);
    }
    fz_catch(ctx){
        fz_rethrow(ctx);
    }
    return time;
}

// used to test server connection
bool ntko_http_get_svrinfo(http_helper *helper, ntko_server_info *svrinfo, 
        ntko_http_response_status *status)
{
    fz_context *ctx = nh_get_context(helper);
    fz_buffer *buf = null;
    fz_xml *xml = null;
    bool isok = false; 
    fz_try(ctx){
        fz_xml *subxml = null;
        unsigned char *data; 
        int size;
        ntko_init_response_status(ctx, status); 

        ntko_do_http_command(helper, &nr_get_serverinfo, svrinfo->rooturl, null);

        buf = nh_reset_status(helper, null);
        size = fz_buffer_get_data(ctx, buf, &data);

        xml = ntko_xml_parse(ctx, data, size); 

        subxml = nxml_find_down(ctx, xml, nxml_ResponseStatus);
        isok = nxml_parse_response_status(ctx, subxml, svrinfo, status); 
        if(isok) {
            subxml = nxml_find_down(ctx, xml, nxml_ResponseInfo);
            isok = nxml_parse_serverinfo(ctx, subxml, svrinfo); 
        } 
    }
    fz_always(ctx) {
        if(buf) fz_drop_buffer(ctx, buf);
        if(xml) fz_drop_xml(ctx, xml);
    }
    fz_catch(ctx){
        fz_rethrow(ctx);
    }
    return isok; 
}

z_list *ntko_http_get_signlist(http_helper *helper, 
        ntko_server_info *svrinfo,
        ntko_user_rights *rights,
        ntko_sign_options *options,
        ntko_http_response_status *status)
{
    fz_context *ctx = nh_get_context(helper);
    fz_buffer *buf = null;
    fz_xml *xml = null;
    z_list *l = null;
    fz_try(ctx){
        fz_xml *subxml = null;
        unsigned char *data; 
        int size;
        ntko_init_response_status(ctx, status); 

        ntko_do_http_command(helper, &nr_get_signlist, svrinfo->rooturl, null);

        buf = nh_reset_status(helper, null);
        size = fz_buffer_get_data(ctx, buf, &data);

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
    }
    fz_always(ctx) {
        if(buf) fz_drop_buffer(ctx, buf);
        if(xml) fz_drop_xml(ctx, xml);
    }
    fz_catch(ctx){
        fz_rethrow(ctx);
    }
    return l; 
}

bool ntko_http_get_user_rights(http_helper *helper, ntko_server_info *svrinfo, ntko_user_rights *right, ntko_sign_options *options, ntko_http_response_status *status) 
{
    fz_context *ctx = nh_get_context(helper);
    fz_buffer *xmlbuf = null;
    fz_xml *xml = null;

    bool isok = false;
    fz_try(ctx) {
        unsigned char *xmldata = null;
        int size = 0;
        ntko_do_http_command(helper, &nr_get_rights, svrinfo->rooturl, null);
        xmlbuf = nh_reset_status(helper, null);
        size = fz_buffer_get_data(ctx, xmlbuf, &xmldata);

        xml = ntko_xml_parse(ctx, xmldata, size);
        fz_xml *subxml = nxml_find_down(ctx, xml, nxml_ResponseStatus);

        isok = nxml_parse_response_status(ctx, subxml, svrinfo, status);
        if(isok) {
            subxml = nxml_find_down(ctx, xml, nxml_ResponseInfo);
            isok = nxml_parse_user_right(ctx, subxml, right);
            if(isok)
                isok = nxml_parse_sign_options(ctx, subxml, options); 
        } 
    }
    fz_always(ctx) {
        if(xmlbuf) fz_drop_buffer(ctx, xmlbuf);
        if(xml) fz_drop_xml(ctx, xml);
    }
    fz_catch(ctx)
        fz_rethrow(ctx);
    return isok;
}

bool ntko_http_dosign_log(http_helper *helper, ntko_server_info *svrinfo, char *log,
       ntko_user_rights *right, ntko_http_response_status *status) 
{
    fz_context *ctx = nh_get_context(helper);
    fz_buffer *xmlbuf = null;
    fz_xml *xml = null;

    bool isok = false;
    fz_try(ctx) {
        unsigned char *xmldata = null;
        int size = 0;
        ntko_do_http_command(helper, &nr_get_rights, svrinfo->queryurl, log);
        xmlbuf = nh_reset_status(helper, null);
        size = fz_buffer_get_data(ctx, xmlbuf, &xmldata);

        xml = ntko_xml_parse(ctx, xmldata, size);
        fz_xml *subxml = nxml_find_down(ctx, xml, nxml_ResponseStatus);
        isok = nxml_parse_response_status(ctx, subxml, svrinfo, status);
        if(isok) {
            subxml = nxml_find_down(ctx, xml, nxml_ResponseInfo);
            isok = nxml_parse_user_right(ctx, subxml, right);
        } 
    }
    fz_always(ctx) {
        if(xmlbuf) fz_drop_buffer(ctx, xmlbuf);
        if(xml) fz_drop_xml(ctx, xml);
    }
    fz_catch(ctx)
        fz_rethrow(ctx);

    return isok;
} 

bool ntko_http_download_esp(http_helper *helper, ntko_server_info *info, ntko_server_espinfo *espinfo, ntko_http_response_status *status)
{
    if(espinfo->data) 
        return true;

    bool isok = false;
    fz_context *ctx = nh_get_context(helper);
    fz_try(ctx) {
        ntko_request req = {espinfo->filename, nrt_get_binary_file, http_get}; 
        ntko_do_http_command(helper, &req, info->rooturl, null);

        if(espinfo->data)
            fz_drop_buffer(ctx, espinfo->data); 

        espinfo->data = nh_reset_status(helper, null);
        if(espinfo->data)
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
            info->sign_svr_name = nxml_att_strdup_nothrow(ctx, subxml, nxml_att_serverName);
            info->server_app_url = nxml_att_strdup_nothrow(ctx, subxml,nxml_att_RserverAppURL); 
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
z_list *ntko_http_get_check_servers(http_helper *helper, ntko_server_info *svrinfo, ntko_http_response_status *status)
{
    fz_context *ctx = nh_get_context(helper);
    fz_xml *xml = null;
    fz_buffer *xmlbuf = null;
    z_list *l = null;
    fz_try(ctx) {
        unsigned char *xmldata = null;
        int size = 0;
        fz_xml *subxml = null;
        bool isok = false;
        ntko_do_http_command(helper, &nr_get_server_changeinfo, svrinfo->rooturl, null);
        xmlbuf = nh_reset_status(helper, null);

        size = fz_buffer_get_data(ctx, xmlbuf, &xmldata);
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
        if(xmlbuf) fz_drop_buffer(ctx, xmlbuf);
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
            info->sign_unid = nxml_att_strdup_nothrow(ctx, subxml, "signUNID");
            info->found = nxml_att_bool(ctx, subxml, "found", false);
            if(!info->found)
                continue;
            info->signer = nxml_att_strdup_nothrow(ctx, subxml, "signer");
            info->sign_name = nxml_att_strdup_nothrow(ctx, subxml, "signName");
            info->sign_user = nxml_att_strdup_nothrow(ctx, subxml, "signUser");
            info->sign_sn = nxml_att_strdup_nothrow(ctx, subxml, "signSN");
            info->ekey_sn = nxml_att_strdup_nothrow(ctx, subxml, "ekeySN");
            info->server_time = nxml_att_strdup_nothrow(ctx, subxml, "serverTime");
            info->app_name = nxml_att_strdup_nothrow(ctx, subxml, "appName");
            info->client_ip = nxml_att_strdup_nothrow(ctx, subxml, "clinetIP");

            subxml = fz_xml_find_next(subxml, "SignsCheckStatus");
        }
    }
    fz_catch(ctx) {
        isok = false;
        fz_rethrow(ctx);
    }
    return isok;
}
        

bool ntko_http_check_sign_one_serverurl(http_helper *helper, ntko_server_info *svrinfo, 
        ntko_http_response_status *status, ntko_check_sign_svrinfo *checksvrinfo,
        z_list *svrsignlist)
{
    fz_context *ctx = nh_get_context(helper);

    fz_xml *xml = null;
    fz_buffer *xmlbuf = null;
    bool isok = false;
    fz_try(ctx) {
        fz_xml *subxml = null;
        unsigned char *xmldata = null;
        int size = 0;
        ntko_do_http_command(helper, &nr_check_sign, svrinfo->rooturl, checksvrinfo->postdata);
        xmlbuf = nh_reset_status(helper, null);
        size = fz_buffer_get_data(ctx, xmlbuf, &xmldata);

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
        if(xmlbuf) fz_drop_buffer(ctx, xmlbuf);
    }
    fz_catch(ctx) {
        fz_rethrow(ctx); 
    }
    return isok;
}

z_list *ntko_http_check_signs(http_helper *helper, ntko_server_info *svrinfo, 
        ntko_http_response_status *status, z_list *signlist)
{
    fz_context *ctx = nh_get_context(helper);

    z_list *svrlist = null; 
    z_list *svr_sign_list = null;
    fz_try(ctx) {
        svrlist = ntko_http_get_check_servers(helper, svrinfo, status);
        init_check_svr_postdata(ctx, svrlist, signlist); 

        z_list_node *node = svrlist->first;
        ntko_check_sign_svrinfo *info = null;
        while(node) {
            info = node->data;
            if(!info)
                continue;

            if(!svr_sign_list)
                svr_sign_list = ntko_svr_signinfo_list_new(ctx);
            ntko_http_check_sign_one_serverurl(helper, svrinfo, status, info, svr_sign_list); 
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
    fz_try(ctx)
        espinfo = fz_malloc_struct(ctx, ntko_server_espinfo);
    fz_catch(ctx)
        fz_rethrow(ctx);
    return espinfo;
}

static
void espinfo_drop(fz_context *ctx, void *data)
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


