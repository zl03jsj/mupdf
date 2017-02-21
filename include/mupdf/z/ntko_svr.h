/*
 * =====================================================================================
 *       Filename:  ntko_helper.h
 *    Description:  
 *        Version:  1.0
 *        Created:  2017/01/25 12时28分14秒
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  YOUR NAME (zl), 
 *   Organization:  ntko.com
 * =====================================================================================
 */
#ifndef ntko_svr_h__
#define ntko_svr_h__

#include <mupdf/fitz.h>
#include "mupdf/z/z_math.h"

typedef unsigned short bool; 
#define true  (unsigned short)1
#define false (unsigned short)0

#ifdef null
#undef null
#endif

#ifdef NULL
#undef NULL
#endif

#define null (void*)0 
#define NULL (void*)0

enum { 
    FZ_NTKO_ERROR_BEGIN = FZ_ERROR_COUNT + 1,
    FZ_ERROR_HTTP_REQUEST,
    FZ_ERROR_XML,
    FZ_NTKO_END
};

typedef enum {
	nrt_get_root_serverinfo     = 1,	// 1:获取根服务器信息 
	nrt_unkown					= 2,
	nrt_login					= 3,	// 3:用户登录。
	nrt_get_signlist			= 4,	// 4:获取服务器软件印章列表
	nrt_get_ekey_useable_state	= 5,	// 5.获取用户ekey可用状态
	nrt_check_right_handsign	= 6,	// 6:检查手写签名权限
	nrt_check_right_keyboradcomment = 7,// 7:检查键盘批注权限	
	nrt_do_sign_log				= 8,	// 8:签章审计日志记录
	nrt_get_binary_file		    = 9,	// 9:二进制文件请求,未使用
	nrt_logout					= 10,	// 10:用户注销请求
	nrt_test_connet_server	    = 11,	// 11:测试服务器连接
	nrt_get_server_time		    = 12,	// 12:获取服务器时间
	nrt_check_sign				= 13,	// 13:检查印章,
	nrt_get_server_changeinfo	= 14,	// 14:获取服务器变更(过期)信息
	nrt_get_right_of_ekeyandsign= 15,	// 15:获取用户ekey及印章的关联权限
	nrt_get_server_comments		= 16,	// 16:获取服务器批注列表
	nrt_set_server_comments		= 17,	// 17:设定服务器批注列表
	nrt_keep_session			= 18,	// 18:保持session
	nrt_check_right_barcode		= 20,	// 20:检查二维码权限
} ntko_request_type;

typedef enum {
    login_code_ok = 0,
    login_code_wrong_namepassword = 1,
    login_code_ekey_must = 2,
    login_code_invalid_ekey = 3,
    login_code_invalid_namepassword = 4,
    login_code_remain_begin = 5,
    login_code_remain_end = 100,
    login_code_server_err_begin
} ntko_login_code;

typedef enum {
    lic_all = 0, lic_office = 1, lic_web_pc = 3,
    lic_web_mobile = 4, lic_pdf_pc = 5, lic_wps = 6, lic_yz = 7
} ntko_product, ntko_license;


typedef enum {
    http_get = 0,
    http_post
} http_submit_type;

struct ntko_request_s {
    char *name;
    ntko_request_type request;
    http_submit_type submit; 
};
typedef struct ntko_request_s ntko_request;

// code: status code of custom defined protocol, after an http request,
// if no exception occur on server, server returned xml formated stream,
// parse xml, we can get this status code, 
// 0 : successed, and operation should continue
// -1: exception occured on server
// othre value: operation should not continue, detail inormation 
// to look at fail_reason.
//
// data :fz_buffer, http response data from sign server,
// if the http status code is 200, data is an xml formated
// buffer, else ignore
//
// sessionid: string, session id get from http response header
struct ntko_http_response_status_s {
    int  code;
    fz_buffer *data;
    char sessionid[64];
    char *fail_reason;
    void *user_param;   
}; 
typedef struct ntko_http_response_status_s ntko_http_response_status;

struct ntko_server_info_s {
    char *queryurl;
    char *rooturl;      // login server url 
    char *servername;
    char *version; 
    int serverid; 
    char *lic_username;
    int lic_count;
    ntko_license lictype; 
}; 
typedef struct ntko_server_info_s ntko_server_info;

struct ntko_user_right_s 
{
	bool permit_handsign;
	bool permit_keyboradSign;
	bool permit_normalSign;
	bool permit_ekeySign;
	bool permit_barCode;
	bool permit_fingerPrint;
	bool permit_multiPage_sign;
	bool permit_multiCrossPage_sign;
	bool permit_doubleCrossPage_sign;
	bool multiPage_sign_need_ekey;       // 多页章是否使用ekey
	bool multiCrossPage_sign_need_ekey;  // 骑缝章是否使用ekey
}; 
typedef struct ntko_user_right_s ntko_user_rights;

typedef enum ntko_handsign_line_mode_e 
{
	nopress = 0,
    imitate_press,
    real_press
} ntko_handsign_line_mode;

typedef enum ntko_sign_comment_vertical_location_e
{
    nscv0 = 0, nscv1, nscv2, nscv3, nscv4,
    nscv5, nscv6, nscv7, nscv8, nscv9, nscva, nscvb
} ntko_sign_comment_vertical_location;

typedef enum ntko_sign_comment_horizontal_location_e 
{
    nsch0 = 0, nsch1, nsch2
} ntko_sign_comment_horizontal_location;

typedef enum ntko_sign_time_veritical_align_e
{ 
    nstva_0 = 0,// "印章内顶部"
    nstva_1,    // "中间偏上"
    nstva_2,    // "中间偏下"
    nstva_3,    // "印章内底部"
    nstva_4,    // "印章下方"
    nstva_5,    // "不显示"
    nstva_6,    // "印章内居中"
    nstva_7     // "自定义位置"
} ntko_sign_time_veritical_align;

typedef enum ntko_sign_time_horizontal_align_e 
{
    nstha_0 = 0, // "印章内顶部"
    nstha_1,     // "中间偏上"
    nstha_2      // "中间偏下"
} ntko_sign_time_horizontal_align;

// nstm = combine first char of each word: ntko sign time mode
typedef enum ntko_sign_time_mode_e
{
    nstm_0 = 0, //二〇〇九年一月二十二日
    nstm_1 = 1, //2009年1月22日
    nstm_2 = 2, //2009-1-22
    nstm_3 = 3, //2009.1.22
} ntko_sign_time_mode;

// 0: Other; 1:Word, 2:Excel,6:WPS,7 ET电子表,5 pdf
// nat = combine first char of each word: ntko application type
typedef enum ntko_app_type_e
{
    nat_other = 0,// "Other Application"
    nat_word,     // "MS Word"
    nat_excel,    // "MS Excel"
    nat_web,      // "HTML 印章"
    nat_pdf,      // "NTKO Pdf"
    nat_wps,      // "WPS 文字"
    nat_etd,      // "WPS 电子表格" 
    nat_yz,       // "永中文字"
} ntko_app_type;

typedef enum ntko_sign_print_mode_e 
{
    invisible = 0,
    normal,
    gray,
} ntko_sign_print_mode;

typedef int ntko_support_ekey_type;

struct ntko_sign_options_s {
    bool check_doc_modification; 
    bool add_comment;
    bool use_certificate;
    int  printmode;
    bool show_option_ui;
    bool lock_sign;
    bool saved;
    bool check_font_change;
    bool sign_add_hand_draw;
    bool sign_add_date;
    char *csp_issure_name;
}; 
typedef struct ntko_sign_options_s ntko_sign_options;


// ntko_server_espinfo
// used to download esp file from server
struct ntko_server_espinfo_s {
    char *signname;
    char *signuser;    
    char *filename;
    fz_buffer *data;
};
typedef struct ntko_server_espinfo_s ntko_server_espinfo;

struct ntko_check_sign_svrinfo_s {
	int serverid;
	char *sign_svr_name;
    char *server_app_url;
    bool used_in_this_doc;
    char *postdata;
};
typedef struct ntko_check_sign_svrinfo_s ntko_check_sign_svrinfo;

typedef struct ntko_server_sign_info_s
{
	char *sign_unid;
	bool found;
	char *signer;
	char *sign_name;
	char *sign_user;
	char *sign_sn;
	char *ekey_sn;
	char *server_time;
	char *app_name;
	char *client_ip;
	int server_id;
} ntko_server_sign_info;

struct ntko_sign_info_s {
    char *sign_unid;
    char *sign_name;
    char *sign_user;
    char *sign_sn;
    char *signer;
    int serverid;
    int signtype;
};
typedef struct ntko_sign_info_s ntko_sign_info;

z_list *ntko_signinfo_list_new(fz_context *ctx);
z_list *ntko_svr_signinfo_list_new(fz_context *ctx);
z_list *ntko_espinfo_list_new(fz_context *ctx);
z_list *ntko_sign_check_svrinfo_list_new(fz_context *ctx); 
/*
    ntko_http_login: do login to ntko sign web server,
    this function throws error, if http request not complete, 
    or get an not expected(must be xml formated, and specified xml tag) http response
    
    username, password: login acount, and it's password

    status: if http request complete(submit, and get an http response), 
    status was set, if login faild, get fail reason from
    status's "failreason" and "code" member.
    or status's "failreason" is null, "code" is 0

    svrinfo: server information, svrinfo->queryurl must be initalize,
    svrinfo->rooturl will be set
    maybe, not sure:svrinfo->name, svrinfo->serverid will be set by this function

    rights: user's right,if login success, the user's rights will be set

    return: true: login success, false, loign failed, failreason stored
    in status
*/
bool ntko_http_login(fz_context *ctx, char *username, char *password,
        ntko_http_response_status *status, ntko_server_info *svrinfo,
        ntko_user_rights *rights); 

bool ntko_http_logoff(fz_context *ctx, ntko_http_response_status *status, ntko_server_info *svrinfo);

/* 
    ntko_http_get_svrtime: get time string from ntko sign server,
    must be "fz_free" 
*/
char *ntko_http_get_svrtime(fz_context *ctx, ntko_server_info *svrinfo,
        ntko_http_response_status *status);

 
/* 
    ntko_http_get_svrinfo:get ntko sign server information.
    svrinfo->version, ->servername, ->lic_username, ->lic_count will be set
*/
bool ntko_http_get_svrinfo(fz_context *ctx, ntko_server_info *svrinfo,
        ntko_http_response_status *status);


/* 
    ntko_http_get_signlist: get user rights, esp information list,
    sign options, from sign server.

    returns a ntko_server_espinfo z_list
*/
z_list *ntko_http_get_esplist(fz_context *ctx, ntko_server_info *svrinfo,
        ntko_user_rights *rights, ntko_sign_options *options,
        ntko_http_response_status *status);

bool ntko_http_get_user_rights(fz_context *ctx, ntko_server_info *svrinfo,
        ntko_user_rights *right, ntko_sign_options *options,
        ntko_http_response_status *status);

/* 
    ntko_http_dosign_log: do sign opration log on sign server

    log: posted form fields Key-Value Pair combined string, formated as:
        "username=???&password=???&signname=???&signuser=???&"  \
        "signsn=???&signunid=???&ekeysn=???&servertime=???&"    \
        "appname=???&cspreleasename=???cspusename=???&"         \
        "signtype=???&signop=???&docfile=???&docinfo=???signpos=???"

    example of normal sign:
*/
bool ntko_http_dosign_log(fz_context *ctx, ntko_server_info *svrinfo, char *log,
       ntko_user_rights *right, ntko_http_response_status *status);

bool ntko_http_download_esp(fz_context *ctx, ntko_server_info *info,
        ntko_server_espinfo *espinfo, ntko_http_response_status *status);

bool ntko_http_check_sign_one_serverurl(fz_context *ctx, ntko_server_info *svrinfo, 
        ntko_http_response_status *status, ntko_check_sign_svrinfo *checksvrinfo,
        z_list *svrsignlist);

/*  
    ntko_http_check_signs:
 
    returns ntko_server_sign_info z_list
*/
z_list *ntko_http_check_signs(fz_context *ctx, ntko_server_info *svrinfo, 
        ntko_http_response_status *status, z_list *signlist);

extern ntko_request nr_get_rootserver;
extern ntko_request nr_login;
extern ntko_request nr_get_signlist;
extern ntko_request nr_get_ekey_useable_state;
extern ntko_request nr_get_rights;
extern ntko_request nr_do_sign_log;
//extern ntko_request nr_get_binary_file;
extern ntko_request nr_logout;
extern ntko_request nr_get_serverinfo;
extern ntko_request nr_get_server_time;
extern ntko_request nr_check_sign;
extern ntko_request nr_get_server_changeinfo;
//extern ntko_request nr_get_right_of_ekeyandsign;
//extern ntko_request nr_get_server_comments;
//extern ntko_request nr_set_server_comments;
//extern ntko_request nr_keep_session;

#endif
