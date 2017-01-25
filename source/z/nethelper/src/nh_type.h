/*
 * =====================================================================================
 *
 *       Filename:  nh_type.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2017/01/24 17时45分52秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <mupdf/fitz.h>

typedef char* u8string;
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
    FZ_NET_ERR_NONE = FZ_ERROR_COUNT + 1,
    FZ_NET_ERR_UNKOWN,
    FZ_NET_ERR_INVALID_PARAMETER,
    FZ_NET_ERR_INVALID_USERNAME,
    FZ_NET_ERR_INVALID_PASSWORD
};


