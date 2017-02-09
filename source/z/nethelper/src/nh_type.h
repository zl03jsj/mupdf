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

#ifndef nh_type_h__
#define nh_type_h__
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
    FZ_NTKO_ERROR_BEGIN = FZ_ERROR_COUNT + 1,
    FZ_ERROR_HTTP_REQUEST,
    FZ_ERROR_XML,
    FZ_NTKO_END
};

#endif
