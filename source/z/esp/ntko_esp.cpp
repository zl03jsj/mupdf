/*
 * =====================================================================================
 *       Filename:  ntko_esp.cpp
 *    Description:  
 *        Version:  1.0
 *        Created:  2017/01/17 16时04分44秒
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  YOUR NAME (), 
 *   Organization:  
 */

#include "mupdf/z/ntko_esp.h"
#include "keygen.h"

#ifdef STD_STRING_CONVERTOR
#include <codecvt>
#endif

int z_is_little_endain() {
    short t = 0x0001;
    char c = ((char*)&t)[0];
    return c==0x01; 
}

#ifndef STD_STRING_CONVERTOR
// windows wchar_t is 2 bytes size
// linux,unix is 4 bytes by default, we can set compiler 
// -fshort-wchar option to let wchar_t to 2 byte size,
// but standard c/c++ library not set this option,
// so, if we set -fshort-wchar, we can not use standard 
// wide char functions
static int
z_ucs2_len(char16_t *ucs2) {
    unsigned int size = 0;
    unsigned short *tmp = (unsigned short*)ucs2; 
    while(tmp[size++]!=0);
    return --size; 
}

static unsigned int 
z_ucs2_change_endain(char16_t *ucs2)
{
    if(!ucs2) return 0;
    unsigned int size = z_ucs2_len(ucs2);
    unsigned int length = size;
    unsigned short *tmp = (unsigned short*)ucs2; 
    while (*tmp && length) { 
        length--;
        unsigned char val_low = *(char*)tmp;
        unsigned char val_high = *(((char*)tmp)+1); 
        *tmp = val_low << 8 | val_high;
        tmp++;
    } 
    return size-length;
}

static int
rune_from_utf16be(int *out, unsigned char *s, unsigned char *end)
{
	if (s + 2 <= end) {
		int a = s[0] << 8 | s[1];
        // https://en.wikipedia.org/wiki/UTF-16
        // U+0000 to U+D7FF and U+E000 to U+FFFF
        // Both UTF-16 and UCS-2 encode code points in this range as single 16-bit code units that are numerically equal to the corresponding code points. These code points in the Basic Multilingual Plane (BMP) are the only code points that can be represented in UCS-2.[citation needed] As of Unicode 9.0 some modern non-latin Asian, Middle-eastern and African scripts fall outside this range.
		if (a >= 0xD800 && a <= 0xDFFF && s + 4 <= end) {
			int b = s[2] << 8 | s[3];
			*out = ((a - 0xD800) << 10) + (b - 0xDC00) + 0x10000;
			return 4;
		}
		*out = a;
		return 2;
	}
	*out = 0xFFFD;
	return 1;
}

int z_ucs2be_to_utf8_len(fz_context *ctx, unsigned char *srcptr, size_t srclen) {
	size_t dstlen = 0; 
	int ucs;
	size_t i;

    i = 0;
    while (i <= srclen) {
        i += rune_from_utf16be(&ucs, srcptr + i, srcptr + srclen);
        dstlen += fz_runelen(ucs);
    }
    return dstlen; 
}

char * z_ucs2be_to_utf8(fz_context *ctx, unsigned char *srcptr, size_t srclen)
{
	char *dstptr, *dst;
	size_t dstlen = 0;
	int ucs;
	size_t i;

    dstlen = z_ucs2be_to_utf8_len(ctx, srcptr, srclen);

    if(0==dstlen) {
        fz_warn(ctx, "ucs2be_to_utf8_len returned 0");
        return NULL;
    }

    dstptr = dst = (char*)fz_malloc(ctx, dstlen + 1); 
    i = 0;
    while (i < srclen)
    {
        i += rune_from_utf16be(&ucs, srcptr + i, srcptr + srclen);
        dstptr += fz_runetochar(dstptr, ucs);
    }
    *dstptr = 0;
	return dst;
}
#endif

#define fz_read_type(ctx, stm, out, type) \
    if(sizeof(type)!=fz_read(ctx, stm, (unsigned char*)out, sizeof(type)))  \
        fz_throw(ctx, FZ_ERROR_GENERIC, "need to read %d count bytes", (int)sizeof(type));

NTKOEspParser::NTKOEspParser(fz_context *_ctx) {
    ctx = _ctx;
    espbuffer = NULL;
    pswOk = false;
    crcOk = false; 
    encoded = false;
    memset(signname, 0, sizeof(signname));
    memset(signuser, 0, sizeof(signname));
    memset(&espHeader, 0, sizeof(NTKOEspHeader)); 
}

NTKOEspParser::~NTKOEspParser() {
    if(espbuffer) {
		if(encoded) encodeData();
        fz_drop_buffer(ctx, espbuffer); 
    }
}

bool NTKOEspParser::open(unsigned char *data, int size, char *password, bool copydata) 
{
    fz_buffer *buff = NULL;
    bool isok = false;
    fz_try(ctx) {
        unsigned char *tmpdata = NULL;
        if(copydata) {
            tmpdata = (unsigned char*)fz_malloc(ctx, size);
            memcmp(tmpdata, data, size);
        }
        else
            tmpdata = data;

        buff = fz_new_buffer_from_data(ctx, tmpdata, size);
        tmpdata = NULL;

        isok = open(buff, password);
    }
    fz_always(ctx) {
        if(buff) fz_drop_buffer(ctx, buff);
    }
    fz_catch(ctx)
        fz_rethrow(ctx);
    return isok;
}

bool NTKOEspParser::open(const char *filename, char *password) {
    fz_stream *stm = NULL;
    fz_buffer *buffer = NULL;
    fz_try(ctx) {
        stm = fz_open_file(ctx, filename);
        fz_seek(ctx, stm, 0, SEEK_SET);
        buffer = fz_read_all(ctx, stm, 1024); 
        if(IsNtkoEspFile(ctx, buffer)) { 
            open(buffer, password);
        }
        else {
            fz_warn(ctx, "invalid esp file flag");
        }
    }
    fz_always(ctx) {
        if(buffer) fz_drop_buffer(ctx, buffer);
        if(stm) fz_drop_stream(ctx, stm);
    }
    fz_catch(ctx) {
        printf("open esp file error:%s\n", ctx->error->message);
    } 
    return pswOk&&crcOk;
} 

bool NTKOEspParser::open(fz_buffer *_buffer, char *password) {
    fz_try(ctx) {
        if(IsNtkoEspFile(ctx, _buffer)) {
            if(espbuffer) {
                fz_drop_buffer(ctx, espbuffer);
                espbuffer = NULL;
            } 
            pswOk = false;
            espbuffer = fz_keep_buffer(ctx, _buffer);
			checkCrcValue();
            if(crcOk) {
                initEspHeader();
                verifyPassword(password);
                if(!pswOk) 
                    fz_warn(ctx, "wrong password."); 
            }
            else
                fz_warn(ctx, "check CrcCode failed.");
        }
        else 
            fz_warn(ctx, "Invalid esp file.");
    }
    fz_catch(ctx) {
        printf("open esp file error:%s\n", ctx->error->message);
    } 

    return pswOk&&crcOk;
}

const NTKOEspHeader* NTKOEspParser::getHeader() {
    return &espHeader;
}

fz_image *NTKOEspParser::getImage() {
    if(!pswOk || !crcOk) return NULL;

    fz_image *image = NULL;
    fz_buffer *imgbuff = NULL;
    fz_try(ctx) {
        imgbuff = getImagedata();
        image = fz_new_image_from_buffer(ctx, imgbuff);
    }
    fz_always(ctx) {
        if(imgbuff) fz_drop_buffer(ctx, imgbuff);
    }
    fz_catch(ctx) {
        fz_rethrow(ctx);
    } 
    return image; 
}

fz_buffer *NTKOEspParser::getImagedata() {
    if(!pswOk || !crcOk) return NULL;

    if(!encoded) 
        encodeData();

    fz_buffer *imgbuff = NULL;
	unsigned char *imagedata = NULL;
	int imageSize = 0;
    fz_try(ctx)
    {
		unsigned char *espdata;
        int totalSize = fz_buffer_get_data(ctx, espbuffer, &espdata);
		
        imageSize = totalSize - NTKOEspFileHeaderSize;
		
        imagedata = (unsigned char*)fz_malloc(ctx, imageSize);
		
        memcpy(imagedata, espdata+NTKOEspFileHeaderSize, imageSize);

        imgbuff = fz_new_buffer_from_data(ctx, imagedata, imageSize); 
    }
    fz_catch(ctx) {
		if(imagedata) fz_free(ctx, imagedata);
        fz_rethrow(ctx); 
    }

    return imgbuff;
}

void NTKOEspParser::initEspHeader() {
    if(!IsNtkoEspFile(ctx, espbuffer)) {
        fz_throw(ctx, FZ_ERROR_GENERIC, "parsing esp failed:not a esp file.");
    } 
    char *data = NULL;
    unsigned int size = 0; 
    fz_stream *stm = NULL;

    fz_try(ctx) {
        encodeData();

        stm = fz_open_buffer(ctx, espbuffer);

        fz_seek(ctx, stm, 0, SEEK_SET);

        fz_read(ctx, stm, (unsigned char*)espHeader.flag, 8);
        fz_read_type(ctx, stm, &espHeader.version,  int);
        fz_read_type(ctx, stm, &espHeader.codepage, int);
        fz_read_type(ctx, stm, &espHeader.crcValue, unsigned int); 
        fz_read(ctx, stm, (byte*)signname, NTKO_MAX_SIGNNAME*2); 
        fz_read(ctx, stm, (byte*)signuser, NTKO_MAX_SIGNUSER*2); 

        unsigned int tmpsize = 0; 

#ifdef STD_STRING_CONVERTOR
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t, 0x10ffff, std::little_endian>,char16_t> ucs2_utf8_cvt; 

        std::string tmp = ucs2_utf8_cvt.to_bytes(signname); 
        tmpsize = fz_mini(tmp.length(), NTKO_MAX_SIGNNAME*2-1); 
        memcpy(espHeader.signname, tmp.data(), tmpsize);
        espHeader.signname[tmpsize] = '\0';
        printf("signname:%s\n", espHeader.signname);

        tmp.clear();

        tmp = ucs2_utf8_cvt.to_bytes(signuser); 
        tmpsize = fz_mini(tmp.length(), NTKO_MAX_SIGNUSER*2-1); 
        memcpy(espHeader.signname, tmp.data(), tmpsize);
        espHeader.signuser[tmpsize] = '\0';
        printf("signuser:%s\n", espHeader.signuser);

        tmp.clear(); 
#else
        // stirng in esp file, use little-endain utf16(ucs2) string
        tmpsize = z_ucs2_change_endain(signname);
        data = z_ucs2be_to_utf8(ctx, (unsigned char*)signname, tmpsize*sizeof(char16_t)); 

        memcpy(espHeader.signname, data, strlen(data));
        fz_free(ctx, data);
        z_ucs2_change_endain(signname);

        tmpsize = z_ucs2_change_endain(signuser);
        data = z_ucs2be_to_utf8(ctx, (unsigned char*)signuser, tmpsize*sizeof(char16_t)); 
        memcpy(espHeader.signuser, data, strlen(data));
        fz_free(ctx, data);
        z_ucs2_change_endain(signuser);
#endif
		
        fz_read(ctx, stm, (byte*)espHeader.md5Val,   NTKO_MAX_MD5VAL); 
        fz_read(ctx, stm, (byte*)espHeader.signSN,   NTKO_MAX_SIGNSN); 
        fz_read(ctx, stm, (byte*)espHeader.reserved, NTKO_MAX_RESERVED);
    }
    fz_always(ctx) {
    }
    fz_catch(ctx)
        fz_rethrow(ctx);

    return;
}

bool NTKOEspParser::checkCrcValue() 
{
    unsigned char* data = NULL;
    unsigned int totalSize, size;
    const unsigned int CRC_OFS = 20;
    fz_try(ctx) {
        totalSize = fz_buffer_get_data(ctx, espbuffer, &data);
        size = totalSize - CRC_OFS;
        data += CRC_OFS; 

        unsigned int oldi = *(unsigned int*)(data-4);
        unsigned int newi = GetCRC32(data, size);
        printf("oldcrc = %d, newcrc = %d\n", oldi, newi);

        crcOk = *(unsigned int*)(data-4)==GetCRC32(data, size);
    }
    fz_catch(ctx) {
        fz_rethrow(ctx);
    }

    return crcOk;
}

static void z_print_hex(char* s, int size) {
    int i = 0;
    while(i<size) {
        printf("%02x ", s[i++]);
        if( (i+1)%4 == 0) printf("\n");
    }
}

bool NTKOEspParser::verifyPassword(char *password) 
{
    unsigned short name_and_user[NTKO_MAX_SIGNNAME+NTKO_MAX_SIGNUSER];
    char md5Buf[NTKO_MAX_MD5VAL];

    memset(name_and_user, 0, sizeof(name_and_user));
    int name_length = std::u16string(signname).length() * sizeof(char16_t);
    int user_length = std::u16string(signuser).length() * sizeof(char16_t);
    byte *tmp = (byte*)name_and_user;
    memcpy(tmp, signname, name_length); 

    tmp += name_length;
    memcpy(tmp, signuser, user_length); 

    CalcKeyLPWSTR(name_and_user, name_length + user_length, md5Buf, password, sizeof(md5Buf), false);

    pswOk = 0==strcmp(espHeader.md5Val, md5Buf);
    return pswOk;
}

bool NTKOEspParser::IsNtkoEspFile(fz_context *ctx, fz_buffer *buffer) {
    bool isEspFile = false;
    unsigned int size = 0;
    unsigned char *data = NULL;

    size = fz_buffer_get_data(ctx, buffer, &data);

    isEspFile = size>NTKOEspFileHeaderSize && 0==memcmp(data, NTKOEspFileFlag, 8);

    if(isEspFile) { 
        // TODO: check current big-endain or little-endain,
        // to fit big-endain architecture, esp file use little-endain
        if( 2!=*(int*)(data + 8) ) { 
            fz_warn(ctx, "only support esp file version 2");
            isEspFile = false;
        } 
    }

    return isEspFile; 
}

NTKOEspParser* NTKOEspParser::create(fz_context *ctx) {
    return new NTKOEspParser(ctx);
}

void NTKOEspParser::encodeData() 
{
    const unsigned int CRC_OFS = 20;
    unsigned char *data = NULL;
    int size = fz_buffer_get_data(ctx, espbuffer, &data); 
    data += CRC_OFS;
    size -= CRC_OFS; 
	while(size--) {
		*data ^= CRC32Table[(size & 0xFF)];
		data++;
	}
    encoded = !encoded;
}

const unsigned char NTKOEspParser::NTKOEspFileFlag[8] = "NTKOESP";
const unsigned int NTKOEspParser::CRC32Table[256] = { 
0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419,
0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4,
0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07,
0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856,
0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a,
0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599,
0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e,
0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed,
0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3,
0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5,
0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010,
0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17,
0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344,
0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a,
0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1,
0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c,
0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef,
0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe,
0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31,
0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b,
0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1,
0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7,
0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66,
0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8,
0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b,
0x2d02ef8d};

unsigned int NTKOEspParser::GetCRC32(unsigned char* data, unsigned int size)
{
    unsigned int crc(0xffffffff);
    // Perform the algorithm on each character
    // in the string, using the lookup table values.
    while(size--)
        crc = (crc >> 8) ^ CRC32Table[(crc & 0xFF) ^ *data++];
    // Exclusive OR the result with the beginning value.
    return crc^0xffffffff;
}
