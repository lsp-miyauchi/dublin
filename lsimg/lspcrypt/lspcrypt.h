//////////////////////////////////////////////////////////////////////
// lspcrypt.h

#ifndef __LSPCRYPT_H__
#define __LSPCRYPT_H__

#define CRYPT_NONE			'n'
#define CRYPT_CP1024		'c'
#define CRYPT_CP256			'2'
#define CRYPT_XOR			'x'

typedef struct tagCRYPTINFO {
	int cbSize;
	LPBYTE pData;
	DWORD dwSize;
	LPCSTR pszCode;
	BYTE crypttype;
	WORD crcSrc;
	WORD crcDst;
	WORD crcCode;
} CRYPTINFO, *LPCRYPTINFO;

#ifdef __cplusplus
extern "C" {                    // Assume C declarations for C++
#endif

extern BOOL WINAPI lspCrypt(CRYPTINFO* pcc);
extern DWORD WINAPI lspCodeCRC(LPCSTR pszCode);

#ifdef __cplusplus
}
#endif


#endif				// __LSPCRYPT_H__
