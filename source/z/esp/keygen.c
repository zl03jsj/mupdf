#include "keygen.h"
#include <string.h> 
#include <locale.h>

#define true  1
#define false 0

static char *toUpperString(char *src) 
{
    char *tmp = src;
    while('\0'!=*tmp){ *tmp = toupper(*tmp); tmp++; }
    return tmp;
}

static void NTKO_MD5_Init(MD5_CTX *c)
{
	c->A=INIT_DATA_A ;
	c->B=INIT_DATA_B ;
	c->C=INIT_DATA_C ;
	c->D=INIT_DATA_D ;
	c->Nl=0;
	c->Nh=0;
	c->num=0;
}

static void md5_block_host_order (MD5_CTX *c, const void *data, int num, int IsForKey)
{
	//const	MD5_LONG *X=(MD5_LONG*)data;
	MD5_LONG *X=(MD5_LONG*)data;
	unsigned long A,B,C,D;
	int		len = num;
	int i = 0;

	c->A^= 0xEDEDEDEDL;
	c->B^= 0xFEFEFEFEL;
	c->C^= 0xCECECECEL;
	c->D^= 0xEEEEEEEEL;


	A=c->A;
	B=c->B;
	C=c->C;
	D=c->D;

	for (;len--;X+= MD5_LBLOCK)
	{
		/* Round 0 */
		R0(A,B,C,D,X[ 0], 7,0xd76aa478L);
		R0(D,A,B,C,X[ 1],12,0xe8c7b756L);
		R0(C,D,A,B,X[ 2],17,0x242070dbL);
		R0(B,C,D,A,X[ 3],22,0xc1bdceeeL);
		R0(A,B,C,D,X[ 4], 7,0xf57c0fafL);
		R0(D,A,B,C,X[ 5],12,0x4787c62aL);
		R0(C,D,A,B,X[ 6],17,0xa8304613L);
		R0(B,C,D,A,X[ 7],22,0xfd469501L);
		R0(A,B,C,D,X[ 8], 7,0x698098d8L);
		R0(D,A,B,C,X[ 9],12,0x8b44f7afL);
		R0(C,D,A,B,X[10],17,0xffff5bb1L);
		R0(B,C,D,A,X[11],22,0x895cd7beL);
		R0(A,B,C,D,X[12], 7,0x6b901122L);
		R0(D,A,B,C,X[13],12,0xfd987193L);
		R0(C,D,A,B,X[14],17,0xa679438eL);
		R0(B,C,D,A,X[15],22,0x49b40821L);


		if(IsForKey)
		{
			for(i=0;i<16;i++)
			{
				X[i] = X[i] ^ 0xABCDEF98L;
			}
		}

		/* Round 1 */
		R1(A,B,C,D,X[ 1], 5,0xf61e2562L);
		R1(D,A,B,C,X[ 6], 9,0xc040b340L);
		R1(C,D,A,B,X[11],14,0x265e5a51L);
		R1(B,C,D,A,X[ 0],20,0xe9b6c7aaL);
		R1(A,B,C,D,X[ 5], 5,0xd62f105dL);
		R1(D,A,B,C,X[10], 9,0x02441453L);
		R1(C,D,A,B,X[15],14,0xd8a1e681L);
		R1(B,C,D,A,X[ 4],20,0xe7d3fbc8L);
		R1(A,B,C,D,X[ 9], 5,0x21e1cde6L);
		R1(D,A,B,C,X[14], 9,0xc33707d6L);
		R1(C,D,A,B,X[ 3],14,0xf4d50d87L);
		R1(B,C,D,A,X[ 8],20,0x455a14edL);
		R1(A,B,C,D,X[13], 5,0xa9e3e905L);
		R1(D,A,B,C,X[ 2], 9,0xfcefa3f8L);
		R1(C,D,A,B,X[ 7],14,0x676f02d9L);
		R1(B,C,D,A,X[12],20,0x8d2a4c8aL);


		if(IsForKey)
		{
			for(i=0;i<16;i++)
			{
				X[i] = X[i] ^ 0x89DCBAFEL;
			}
		}

		/* Round 2 */
		R2(A,B,C,D,X[ 5], 4,0xfffa3942L);
		R2(D,A,B,C,X[ 8],11,0x8771f681L);
		R2(C,D,A,B,X[11],16,0x6d9d6122L);
		R2(B,C,D,A,X[14],23,0xfde5380cL);
		R2(A,B,C,D,X[ 1], 4,0xa4beea44L);
		R2(D,A,B,C,X[ 4],11,0x4bdecfa9L);
		R2(C,D,A,B,X[ 7],16,0xf6bb4b60L);
		R2(B,C,D,A,X[10],23,0xbebfbc70L);
		R2(A,B,C,D,X[13], 4,0x289b7ec6L);
		R2(D,A,B,C,X[ 0],11,0xeaa127faL);
		R2(C,D,A,B,X[ 3],16,0xd4ef3085L);
		R2(B,C,D,A,X[ 6],23,0x04881d05L);
		R2(A,B,C,D,X[ 9], 4,0xd9d4d039L);
		R2(D,A,B,C,X[12],11,0xe6db99e5L);
		R2(C,D,A,B,X[15],16,0x1fa27cf8L);
		R2(B,C,D,A,X[ 2],23,0xc4ac5665L);


		if(IsForKey)
		{
			for(i=0;i<16;i++)
			{
				X[i] = X[i] ^ 0xCB89EDFAL;
			}
		}

		/* Round 3 */
		R3(A,B,C,D,X[ 0], 6,0xf4292244L);
		R3(D,A,B,C,X[ 7],10,0x432aff97L);
		R3(C,D,A,B,X[14],15,0xab9423a7L);
		R3(B,C,D,A,X[ 5],21,0xfc93a039L);
		R3(A,B,C,D,X[12], 6,0x655b59c3L);
		R3(D,A,B,C,X[ 3],10,0x8f0ccc92L);
		R3(C,D,A,B,X[10],15,0xffeff47dL);
		R3(B,C,D,A,X[ 1],21,0x85845dd1L);
		R3(A,B,C,D,X[ 8], 6,0x6fa87e4fL);
		R3(D,A,B,C,X[15],10,0xfe2ce6e0L);
		R3(C,D,A,B,X[ 6],15,0xa3014314L);
		R3(B,C,D,A,X[13],21,0x4e0811a1L);
		R3(A,B,C,D,X[ 4], 6,0xf7537e82L);
		R3(D,A,B,C,X[11],10,0xbd3af235L);
		R3(C,D,A,B,X[ 2],15,0x2ad7d2bbL);
		R3(B,C,D,A,X[ 9],21,0xeb86d391L);

		if(IsForKey)
		{
			for(i=0;i<16;i++)
			{
				X[i] = X[i] ^ 0xEFDACBFEL;
			}
		}

		A = c->A += A;
		B = c->B += B;
		C = c->C += C;
		D = c->D += D;
	}
}

static void md2Char(unsigned char *md, char *buf)
{
	int i;
    printf("md5 value:\n");
	for (i=0; i<MD5_DIGEST_LENGTH; i++){
        sprintf(&(buf[i*2]), "%02X", md[i]); 
        printf("%02X", md[i]);
    }
    printf("\n");
}

static int _isblank( unsigned char c )
{
	return (c==0x20) || ( (0x09<=c) && (c<=0x0D) );
}

/*-----------------------------------------
*	instr:	string to generate key for;	Max	245(256-10)	bytes len
*	outstr:	generated key; Min 33 bytes	len
*	return:	0 for success, 1 for error
*-----------------------------------------*/ 
int CalcKey(char *instr,char *outstr,const char *keypadding,int keylen,int IsForKey)
{
	// format input data

	unsigned char inbuf[256]; //input buffer
	MD5_CTX c;
	unsigned char m[MD5_DIGEST_LENGTH];
	unsigned char *p;
	unsigned char *pt; // for triming
	size_t tlen; // for trimming
	size_t blocks; //cipher blocks
	
	p = m;
	tlen = strlen(instr);

	if ( ( tlen > MAXSERVERNAME_LENGTH) || ( keylen < MINKEYBUF_LENGTH) )
	{
		strcpy(outstr,"");
		return false;
	} 
    
    memset(inbuf, 0, sizeof(inbuf));
    memset(outstr, 0, sizeof(inbuf));

	strcpy((char*)inbuf,instr);
	
	//trim any blanks
	pt = inbuf;

	// do ltrim()
	while ( _isblank(*pt)) pt++;
	tlen = strlen((char*)pt);
    memmove(inbuf, pt, tlen);
	memset(inbuf + tlen, 0, sizeof(inbuf)-tlen);
	
	// do rtrim()
	if ( tlen > 0 )
	{
		pt = inbuf + tlen;
		while ( _isblank(*(--pt)) )
		{
			*pt = 0;
		}	
	}
	// end trimming
	tlen = strlen((char*)inbuf);
	if (tlen == 0 ) 
	{
		strcpy(outstr,"");		
		return false;
	}

	strcat((char*)inbuf, keypadding); //padding info
    toUpperString((char*)inbuf); 

	tlen = strlen((char*)inbuf);

	blocks = tlen /MD5_CBLOCK + 1;

	NTKO_MD5_Init(&c);
	md5_block_host_order(&c,(const void*)inbuf,(int)blocks,IsForKey);

	MD5_MAKE_STRING(&c,p);

	md2Char(m,outstr);

	return true;
}	 

int CalcKeyLPWSTR(unsigned short *instr, unsigned int inbuflen, char *outstr, const char *keypadding,int keylen, int IsForKey)
{
	// format input data

	unsigned char inbuf[MD5_CBLOCK*8]; //input buffer
	MD5_CTX c;
	unsigned char m[MD5_DIGEST_LENGTH];
	unsigned char *p;

	size_t tlen; // for trimming
	size_t blocks; //cipher blocks
	int keyPaddingLen = 0;
	unsigned int md5BufLen = 0;
	
	p = m;

	md5BufLen = sizeof(inbuf);

    memset(m, 0, sizeof(m));

	keyPaddingLen = strlen(keypadding);

	if ( ( (inbuflen +keyPaddingLen)> md5BufLen) || (inbuflen==0)|| ( keylen < MINKEYBUF_LENGTH) )
	{
		strcpy(outstr,"");
		return false;
	}
	    
	memset((void*)inbuf, 0, md5BufLen); //initialize buffer
	memset((void*)outstr, 0, keylen); //initialize out key buffer

    memcpy(inbuf, instr, inbuflen);
	if(keyPaddingLen)
	{
		memcpy(inbuf+inbuflen,keypadding,keyPaddingLen);
	}

	tlen = inbuflen+keyPaddingLen;

	blocks = tlen /MD5_CBLOCK + 1;

	NTKO_MD5_Init(&c);
	md5_block_host_order(&c,(const void*)inbuf,(int)blocks,IsForKey);

	MD5_MAKE_STRING(&c,p);

	md2Char(m,outstr);

	return true;
}
