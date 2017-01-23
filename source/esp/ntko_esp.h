/*
 * =====================================================================================
 *       Filename:  ntko_esp.h
 *    Description:  parsing ntko security signature file(.esp) file.
 *        Version:  1.0
 *        Created:  2017/01/17 15时47分50秒
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  YOUR NAME (zl), 
 *   Organization:  ntko.com
 */
#ifndef ntko_esp_h__
#define ntko_esp_h__ 

#include "mupdf/fitz.h"
// stdlib c++11 support char16_t 
#include <iostream>

#define NTKO_ESP_HEADER_FLAG "NTKOESP"

#define NTKO_MAX_SIGNNAME       100
#define NTKO_MAX_SIGNSN         52
#define NTKO_MAX_RESERVED       52
#define NTKO_MAX_SIGNUSER       100
#define NTKO_MAX_MD5VAL         40
#define NTKO_MAX_PASSWORD       32
#define NTKO_MIN_PASSWORD       6
#define NTKO_MAXEKEY_PASSWORD   32
#define NTKO_MINEKEY_PASSWORD   4

// define this marcro to use std c++11 library function
// to convert ucs2 string to utf8
// #define STD_STRING_CONVERTOR 
#ifdef STD_STRING_CONVERTOR
#pragma message("convert ucs2 to utf8 use stdc++11 library function")
#else
#pragma message("not use stdc++11 library function to convert ucs2 to utf8")
#endif 
typedef unsigned char byte; 
#pragma pack(push, NTKOEspHeader__, 1)
typedef struct NTKOEspHeader_s
{
	char flag[8];  //should be "NTKOESP\0"
	int  version;  //should be 2
	int  codepage; //WCHAR编码的CODEPAGE, use this member on windows
	unsigned int crcValue;
	char signname[NTKO_MAX_SIGNNAME*2];
	char signuser[NTKO_MAX_SIGNUSER*2];
	char md5Val[NTKO_MAX_MD5VAL]; //name & user cated md5 value
	char signSN[NTKO_MAX_SIGNSN];
	char reserved[NTKO_MAX_RESERVED];
} NTKOEspHeader;
#pragma pack(pop, NTKOEspHeader__)

class NTKOEspParser {
public:
    ~NTKOEspParser(); 
    bool open(const char *file, char *password); 
    bool open(fz_buffer* _espbuffer, char *password);
    bool verify(char *password);
    inline bool isValid() {return pswOk;}
    const NTKOEspHeader* getHeader();
    bool verifyPassword(char *password); 
    fz_image *getImage();
    fz_buffer *getImagedata();

    static NTKOEspParser *create(fz_context *ctx);
    static bool IsNtkoEspFile(fz_context *ctx, fz_buffer *buffer);
private:
    NTKOEspParser(fz_context *_ctx); 
    void initEspHeader();
    bool checkCrcValue();
    static void encodeData(unsigned char *data, unsigned int size);
    static unsigned int GetCRC32(unsigned char* data, unsigned int size);
private:
    fz_context *ctx = NULL;
    fz_buffer *espbuffer = NULL;
    NTKOEspHeader espHeader;
    char16_t signname[NTKO_MAX_SIGNNAME];
    char16_t signuser[NTKO_MAX_SIGNUSER];
    bool pswOk = false;
    bool crcOk = false;

    static const unsigned int CRC32Table[256];
    static const int NTKOEspFileHeaderSize = 564;
    static const unsigned char NTKOEspFileFlag[8];
};


#endif
