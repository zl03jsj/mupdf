/*
 * =====================================================================================
 *       Filename:  ntko_helper.h
 *    Description:  
 *        Version:  1.0
 *        Created:  2017/01/25 12时28分14秒
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  YOUR NAME (), 
 *   Organization:  
 * =====================================================================================
 */
#ifndef ntko_helper__
#define ntko_helper__

#include "net_helper.h"

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

struct ntko_request_s {
    ntko_request_type type;
    nh_submit_method method; 
};
typedef struct ntko_request_s ntko_request;

extern ntko_request nr_get_serverinfo;
extern ntko_request nr_login;
extern ntko_request nr_get_sign;
extern ntko_request nr_get_ekey_useable_state;
extern ntko_request nr_check_right_han;
extern ntko_request nr_check_right_keyboradcommen;
extern ntko_request nr_do_sign;
extern ntko_request nr_get_binary_file;
extern ntko_request nr_logout;
extern ntko_request nr_test_con;
extern ntko_request nr_get_server_time;
extern ntko_request nr_check_sign;
extern ntko_request nr_get_server_changeinfo;
extern ntko_request nr_get_right_of_ekeyan;
extern ntko_request nr_get_server_comments;
extern ntko_request nr_set_server_comments;
extern ntko_request nr_keep_session;
extern ntko_request nr_check_right_barcode; 

#endif
