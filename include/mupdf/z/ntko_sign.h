/*
 * =====================================================================================
 *
 *       Filename:  ntko_sign.h
 *    Description:  *
 *        Version:  1.0
 *        Created:  2017/02/27 10时57分44秒
 *       Revision:  none
 *       Compiler:  gcc *
 *         Author:  YOUR NAME (), 
 *   Organization:  *
 * =====================================================================================
 */
#ifndef ntko_sign_h__
#define ntko_sign_h__

//
struct ntko_sign_info_s {
    char *sign_sn;   // for esp file, if not, equals to unid
    char *sign_unid; // unid is create when successfully add an signature
    char *sign_name;
    char *sign_user;
    char *signer;
    int serverid;
    int signtype;
};
typedef struct ntko_sign_info_s ntko_sign_info;

typedef enum ntko_sign_print_mode_e 
{
    print_invisible = 0,
    print_normal,
    print_gray,
} ntko_sign_print_mode;

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

#endif
