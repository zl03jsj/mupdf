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

ntko_request nr_get_root_serverinfo = {nrt_get_root_serverinfo,get};
ntko_request nr_login = {nrt_login,post};
ntko_request nr_get_signlist = {nrt_get_signlist,get};
ntko_request nr_get_ekey_useable_state = {nrt_get_ekey_useable_state,get};
ntko_request nr_check_right_handsign = {nrt_check_right_handsign,get};
ntko_request nr_check_right_keyboradcomment = {nrt_check_right_keyboradcomment,get};
ntko_request nr_do_sign_log = {nrt_do_sign_log,get};
ntko_request nr_get_binary_file = {nrt_get_binary_file,get};
ntko_request nr_logout = {nrt_logout,get};
ntko_request nr_test_connet_server = {nrt_test_connet_server,get};
ntko_request nr_get_server_time = {nrt_get_server_time,get};
ntko_request nr_check_sign = {nrt_check_sign,get};
ntko_request nr_get_server_changeinfo = {nrt_get_server_changeinfo,get};
ntko_request nr_get_right_of_ekeyandsign = {nrt_get_right_of_ekeyandsign,get};
ntko_request nr_get_server_comments = {nrt_get_server_comments,get};
ntko_request nr_set_server_comments = {nrt_set_server_comments,get};
ntko_request nr_keep_session = {nrt_keep_session,get};
ntko_request nr_check_right_barcode = {nrt_check_right_barcode,get};


