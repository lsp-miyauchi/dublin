////////////////////////////////////////////////////////////////////////
// base64.cpp

#include "stdafx.h"


////////////////////////////////////////////////////////////////////////
//

static const BYTE B64[] ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
static const BYTE b64rconv[] = {
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	//	00-15
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	//	16-31
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62,  0,  0,  0, 63,	//	32-47
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0,	//	48-63
   0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 	//	64-79
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,  0,  0,  0,  0,	//	80-95
   0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,	//	96-111
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,  0,  0,  0,  0,  0,	//	112-127
};

////////////////////////////////////////////////////////////////////////
//

static inline BYTE Val(char ch)
{
//@	return (ch == '=' ? 0 : strchr((char*)B64, ch) - (char*)B64);
	return (ch >= 0 && ch < 128) ? b64rconv[ch] : 0;
}

////////////////////////////////////////////////////////////////////////
//

BYTE* b64Decode(const char* inptr, int* plen)
{
	if (inptr == NULL) return NULL;
	size_t len = strlen(inptr);
	BYTE* outptr = (BYTE*)calloc(1, len + 1);
	size_t i;
	int j = 0;
	for (i = 0; i < len; i += 4) {
		char a = *inptr++;
		char b = *inptr++;
		char c = *inptr++;
		char d = *inptr++;
		outptr[j]   = (BYTE)( (Val(a) << 2) | ((Val(b) & 0x30) >> 4));
		outptr[j + 1] = (BYTE)( ((Val(b) & 0x0f) << 4) | ((Val(c) & 0x3c) >> 2));
		outptr[j + 2] = (BYTE)( ((Val(c) & 0x03) << 6) | (Val(d) & 0x3f));
		j += 3;
	}
	outptr[j] = 0;
	*plen = j;
	return outptr;
}

////////////////////////////////////////////////////////////////////////
//

char* b64Encode(const BYTE* inptr, int size)
{
	size_t memsize = size * 2 + 2;
	char* outptr = (char*)malloc(memsize);
	BYTE* dptr = new BYTE[memsize];
	memset(dptr, 0, memsize);
	memcpy(dptr, inptr, size);
	int j = 0;
	for (int i = 0; i < size; i += 3) {
		outptr[j]     = (char)(0 == (dptr[i] >> 2)) ? 'A' : ( B64[ dptr[i] >> 2 ]);
		outptr[j + 1] = (char)(0 == ((dptr[i] & 0x03) << 4 | (dptr[i + 1] >> 4))) ? 'A' : ( B64[ (dptr[i] & 0x03) << 4 | (dptr[i + 1] >> 4) ]);
		outptr[j + 2] = (char)(0 == ((dptr[i + 1] & 0x0f) << 2 | (dptr[i + 2] >> 6))) ? 'A' : ( B64[ (dptr[i + 1] & 0x0f) << 2 | (dptr[i + 2] >> 6)]);
		outptr[j + 3] = (char)(0 == (dptr[i + 2] & 0x3f)) ? 'A' : ( B64[ dptr[i + 2] & 0x3f]);
		j += 4;
	}
	outptr[j] = 0;
	delete [] dptr;
	return outptr;
}
