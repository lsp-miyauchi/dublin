/////////////////////////////////////////////////////////////////////////
// utfconvert.cpp

#include "stdafx.h"
#include "cab.h"

/* -------------------------------------------------------------------
Unicode characters with values 0x0000 through 0x007F are
represented by a single byte of the same value.

The first byte emitted for Unicode characters 0x0080 through 0x07FF is
	byte1 = 0xC0+(unicodevalue >> 6),
	byte2 = 0x80+(unicodevalue & 0x003F).

Unicode characters 0x0800 through 0xFFFF are represented by 
	byte1 = 0xE0+(unicodevalue >> 12), 
	byte2 = 0x80+((unicodevalue >> 6) & 0x3F),
	byte3 = 0x80+(unicodevalue & 0x3F).
------------------------------------------------------------------- */


int EncodeWStrToUTF(const WCHAR* widestr, int widelen, BYTE* utfstr, int utflen)
{
	if (!widestr || utflen < 0 || (!utfstr && utflen > 0)) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}

	if (widelen < 0)
		widelen = (int)wcslen(widestr);

	BYTE* baseptr = new BYTE[widelen * 3 + 8];
	BYTE* pb = baseptr;
	while (widelen-- > 0) {
		WCHAR wch = *widestr++;
		if (!wch) break;

		if (wch < 0x0080)
			*pb++ = (BYTE)wch;
		else if (wch < 0x0800) {
			*pb++ = (BYTE)(0xC0 + (wch >> 6));
			*pb++ = (BYTE)(0x80 + (wch & 0x3F));
		}
		else {
			*pb++ = (BYTE)(0xE0 + (wch >> 12));
			*pb++ = (BYTE)(0x80 + ((wch >> 6) & 0x3F));
			*pb++ = (BYTE)(0x80 + (wch & 0x3F));
		}
	}
	*pb = 0;
	int convlen = (int)strlen((LPCSTR)baseptr);

	if (utflen > 0) {
		if (utflen < convlen) {
			SetLastError(ERROR_INSUFFICIENT_BUFFER);
			convlen = 0;
		}
		else
			memcpy(utfstr, baseptr, convlen + 1);
	}
	delete [] baseptr;

	return convlen;
}

int EncodeStrToUTF(const char* str, int len, BYTE* utfstr, int utflen)
{
	WCHAR wsz[MAX_PATH * 2];
	MultiByteToWideChar(CP_ACP, 0, str, len, wsz, ARRAYSIZE(wsz));
	return EncodeWStrToUTF(wsz, -1, utfstr, utflen);
}

int DecodeUTFToWStr(const BYTE* utfstr, int utflen, WCHAR* widestr, int widelen)
{
	if (!utfstr || widelen < 0 || (!widestr && widelen > 0)) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}

	if (utflen < 0)
		utflen = (int)strlen((LPCSTR)utfstr);

	WCHAR* baseptr = new WCHAR[utflen + 8];
	WCHAR* pw = baseptr;
	while (utflen-- > 0) {
		BYTE cb = *utfstr++;
		if (!cb) break;

		if (cb >= 0xE0) {
			// 3bytes order
			*pw = (cb - 0xE0) << 12;
			*pw += (*utfstr++ - 0x80) << 6;
			*pw += (*utfstr++ - 0x80);
		}
		else if (cb >= 0xC0) {
			// 2bytes order
			*pw = (cb - 0xC0) << 6;
			*pw += (*utfstr++ - 0x80);
		}
		else
			*pw = cb;

		pw++;
	}
	*pw = 0;
	int convlen = (int)wcslen(baseptr);

	if (widelen > 0) {
		if (widelen < convlen) {
			SetLastError(ERROR_INSUFFICIENT_BUFFER);
			convlen = 0;
		}
		else
			memcpy(widestr, baseptr, (convlen + 1) * sizeof(WCHAR));
	}
	delete [] baseptr;

	return convlen;
}

int DecodeUTFToStr(const BYTE* utfstr, int utflen, char* str, int len)
{
	WCHAR wsz[MAX_PATH * 2];
	DecodeUTFToWStr(utfstr, utflen, wsz, ARRAYSIZE(wsz));
	return WideCharToMultiByte(CP_ACP, 0, wsz, -1, str, len, NULL, NULL);
}

BOOL IsUTFEncoded(const BYTE* utfstr)
{
	while (utfstr && *utfstr) {
		if (*utfstr >= 0x80)
			return TRUE;
		utfstr++;
	}
	return FALSE;
}
