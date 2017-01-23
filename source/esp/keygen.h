/*--------------------------------------------------------------------
*
*		File:      keygen.h
*
*		license key generate algorithm def
*
*		Copyright (c) 2003 David.Liu 
*
*-------------------------------------------------------------------*/

#ifndef KEYGEN_H
#define KEYGEN_H


#define MAXSERVERNAME_LENGTH 239
#define MINKEYBUF_LENGTH	33

// unsigned long has diff size on 32/64bit architecture
// replace with unsigned int
// #define MD5_LONG unsigned long 
#define MD5_LONG unsigned int

#define MD5_CBLOCK	64
#define MD5_LBLOCK	(MD5_CBLOCK/4)
#define MD5_DIGEST_LENGTH 16

#define INIT_DATA_A (unsigned long)0x8AA8CEECL//0x67452301L
#define INIT_DATA_B (unsigned long)0x11335577L//0xefcdab89L
#define INIT_DATA_C (unsigned long)0x56741230L//0x98badcfeL
#define INIT_DATA_D (unsigned long)0xFEDCBA98L//0x10325476L;//

typedef struct
{
	MD5_LONG A,B,C,D;
	MD5_LONG Nl,Nh;
	MD5_LONG data[MD5_LBLOCK];
	int num;
} MD5_CTX;

#define ROTATE(a,n)  (((a)<<(n))|(((a)&0xffffffff)>>(32-(n))))

#define	F(b,c,d)	((((c) ^ (d)) & (b)) ^ (d))
#define	G(b,c,d)	((((b) ^ (c)) & (d)) ^ (c))
#define	H(b,c,d)	((b) ^ (c) ^ (d))
#define	I(b,c,d)	(((~(d)) | (b)) ^ (c))

#define R0(a,b,c,d,k,s,t) { \
	a+=((k)+(t)+F((b),(c),(d))); \
	a=ROTATE(a,s); \
	a+=b; };\

#define R1(a,b,c,d,k,s,t) { \
	a+=((k)+(t)+G((b),(c),(d))); \
	a=ROTATE(a,s); \
	a+=b; };

#define R2(a,b,c,d,k,s,t) { \
	a+=((k)+(t)+H((b),(c),(d))); \
	a=ROTATE(a,s); \
	a+=b; };

#define R3(a,b,c,d,k,s,t) { \
	a+=((k)+(t)+I((b),(c),(d))); \
	a=ROTATE(a,s); \
	a+=b; };

#define HOST_l2c(l,c)	(*((c)++)=(unsigned char)(((l)    )&0xff),	\
	*((c)++)=(unsigned char)(((l)>> 8)&0xff),	\
	*((c)++)=(unsigned char)(((l)>>16)&0xff),	\
	*((c)++)=(unsigned char)(((l)>>24)&0xff),	\
	l)

#define	MD5_MAKE_STRING(c,s)	do {	\
	unsigned long ll;		\
	ll=(c)->A; HOST_l2c(ll,(s));	\
	ll=(c)->B; HOST_l2c(ll,(s));	\
	ll=(c)->C; HOST_l2c(ll,(s));	\
	ll=(c)->D; HOST_l2c(ll,(s));	\
	} while (0)

/*------------------------------------------
*	instr:	string to generate key for; Max 245(256-10) bytes len
*	outstr:	generated key; Min 33 bytes len
*	return:	0 for success, 1 for error
*-----------------------------------------*/
#if defined(__cplusplus)
extern "C"
{
#endif

#include <wchar.h>
int CalcKey(char *instr,char *outstr,const char *keypadding,int keylen, int IsForKey);
int CalcKeyLPWSTR(unsigned short *instr, unsigned int inbuflen, char *outstr, const char *keypadding,int keylen, int IsForKey);

#if defined(__cplusplus)
}
#endif 


#endif
