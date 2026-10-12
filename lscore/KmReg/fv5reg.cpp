//////////////////////////////////////////////////////////////////////
// regnum.cpp

#include <windows.h>
#include <limits.h>
#include "debug.h"

#define LODWORD(a)				(*((DWORD*)&(a)))
#define HIDWORD(a)				(*(((DWORD*)&(a)) + 1))
#define SETDWORDLONG(a,b,c)		(*((DWORD*)&(a))=(b),*(((DWORD*)&(a))+1)=(c))


#define BLOWFISH_ENCRYPT
#ifdef BLOWFISH_ENCRYPT
	#include "blowfish.h"
#endif

/* ##########################################

80bit --> 80 / 5 = 16ï∂éö

êÊì™64bitÇÕ blowfish Ç≈à√çÜâªÇ∑ÇÈ

|<----------- à√çÜâª 64bit -------------------------------------------->|<-- 36bit ----------------------------->|
|XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX|********|********|********|********|YYYYYYYY|YYYYYYYY|YYYYYYYY|YYYYYYYY|YYYY|
|-- 0 ---|-- 1 ---|-- 2 ---|-- 3 ---|-- 4 ---|-- 5 ---|-- 6 ---|-- 7 ---|-- 8 ---|-- 9 ---|-- 10 --|-- 11 --|- 12|
|11111222|22333334|44445555|56666677|77788888|99999000|00AAAAAB|BBBBCCCC|CDDDDDEE|EEEFFFFF|00000111|11222223|3333|
|-- 0 ---|-- 1 ---|-- 2 ---|-- 3 ---|-- 4 ---|-- 5 ---|-- 6 ---|-- 7 ---|-- 8 ---|-- 9 ---|-- 10 --|-- 11 --|- 12|
|cccccccc|cccccccc| Å©16 crc        |        |        |        |        |        |        |        |        |    |
|        |        |ssssssss|ssssssss|s  Å©17 seq_1    |        |        |        |        |        |        |    |
|        |        |        |        | ddddddd|ddddddd Å©14 date_1       |        |        |        |        |    |
|        |        |        |        |        |       p|pppppp  Å©7 prx_1         |        |        |        |    |
|        |        |        |        |        |        |      dd|dddddddd|dddd  Å©14 date_2|        |        |    |
|        |        |        |        |        |        |        |  7 prx_2 Å® pppp|ppp     |        |        |    |
|        |        |        |        |        |        |        |        17 seq_2 Å®  sssss|ssssssss|ssss    |    |
|        |        |        |        |        |        |        |        |        |        |        |    **  |    |
|        |        |        |        |        |        |        |        |        |        |        |      **|*   |
|        |        |        |        |        |        |        |        |        |        |         3 idx Å®  iii|
|<----------- à√çÜâª 64bit -------------------------------------------->|<-- 36bit ----------------------------->|

Seq2 ÇÕ Date2 Çâ¡Ç¶ÇΩílÇ∆Ç∑ÇÈ


############################################ */

///////////////////////////////////////////////////////////////
// xxDDDDDMMMMYYYYY
// 0011111111111111
// mask d=0x3e00
//      m=0x01e0
//      y=0x001f
//
#define K2E_DAY(d)			(((d) & 0x3e00) >> 9)
#define K2E_MON(d)			(((d) & 0x01e0) >> 5)
#define K2E_YEAR(d)			(((d) & 0x001f))
#define K2C_DAY(d)			(((d) << 9) & 0x3e00)
#define K2C_MON(d)			(((d) << 5) & 0x01e0)
#define K2C_YEAR(d)			(((d) & 0x001f))

// xxMMMMYYYYYDDDDD
// 0011111111111111
// mask d=0x001f
//      m=0x3c00
//      y=0x03e0
//
#define L2E_DAY(d)			(((d) & 0x001f))
#define L2E_MON(d)			(((d) & 0x3c00) >> 10)
#define L2E_YEAR(d)			(((d) & 0x03e0) >> 5)
#define L2C_DAY(d)			(((d) & 0x001f))
#define L2C_MON(d)			(((d) << 10) & 0x3c00)
#define L2C_YEAR(d)			(((d) << 5) & 0x03e0)


#define LODWORD(a)				(*((DWORD*)&(a)))
#define HIDWORD(a)				(*(((DWORD*)&(a)) + 1))
#define SETDWORDLONG(a,b,c)		(*((DWORD*)&(a))=(b),*(((DWORD*)&(a))+1)=(c))

extern WORD crc16(int n, BYTE* pcb);

/////////////////////////////////////////////////////////////////////
//

static char sszEncBuff[128];


/////////////////////////////////////////////////////////////////////
//

_inline WORD wordrot_left(WORD w, int bits)
{
	DWORD dw = ((DWORD)w) << bits;
	return (LOWORD(dw) | HIWORD(dw));
}

_inline WORD wordrot_right(WORD w, int bits)
{
	DWORD dw = (MAKELONG(0, w)) >> bits;
	return (HIWORD(dw) | LOWORD(dw));
}

_inline DWORD dwordrot_left(DWORD dw, int bits)
{
	DWORDLONG dl = Int64ShllMod32((DWORDLONG)dw, bits);
	return (LODWORD(dl) | HIDWORD(dl));
}

_inline DWORD dwordrot_right(DWORD dw, int bits)
{
	DWORDLONG dl = SETDWORDLONG(dl, 0, dw);
	dl = Int64ShrlMod32(dl, bits);
	return (HIDWORD(dl) | LODWORD(dl));
}

BYTE __fastcall byte_rangerot_left(BYTE w, int range, int rot)
{
	BYTE chkbit = 1;
	BYTE mask = 1;
	while (--range > 0) { chkbit <<= 1; mask = (mask << 1) | 1; }

	BYTE save = w & (0xff^mask);
	w &= mask;
	while (rot > 0) {
		BOOL flg = (w & chkbit);
		w <<= 1;
		if (flg) w |= 1;
		rot--;
	}
	return (save | (w & mask));
}

BYTE __fastcall byte_rangerot_right(BYTE w, int range, int rot)
{
	BYTE chkbit = 1;
	BYTE mask = 1;
	while (--range > 0) { chkbit <<= 1; mask = (mask << 1) | 1; }

	BYTE save = w & (0xffff^mask);
	w &= mask;
	while (rot > 0) {
		BOOL flg = (w & 1);
		w >>= 1;
		if (flg) w |= chkbit;
		rot--;
	}
	return (save | (w & mask));
}

WORD __fastcall word_rangerot_left(WORD w, int range, int rot)
{
	WORD chkbit = 1;
	WORD mask = 1;
	while (--range > 0) { chkbit <<= 1; mask = (mask << 1) | 1; }

	WORD save = w & (0xffff^mask);
	w &= mask;
	while (rot > 0) {
		BOOL flg = (w & chkbit);
		w <<= 1;
		if (flg) w |= 1;
		rot--;
	}
	return (save | (w & mask));
}

WORD __fastcall word_rangerot_right(WORD w, int range, int rot)
{
	WORD chkbit = 1;
	WORD mask = 1;
	while (--range > 0) { chkbit <<= 1; mask = (mask << 1) | 1; }

	WORD save = w & (0xffff^mask);
	w &= mask;
	while (rot > 0) {
		BOOL flg = (w & 1);
		w >>= 1;
		if (flg) w |= chkbit;
		rot--;
	}
	return (save | (w & mask));
}

DWORD __fastcall dword_rangerot_left(DWORD w, int range, int rot)
{
	DWORD chkbit = 1;
	DWORD mask = 1;
	while (--range > 0) { chkbit <<= 1; mask = (mask << 1) | 1; }

	DWORD save = w & (0xffffffff^mask);
	w &= mask;
	while (rot > 0) {
		BOOL flg = (w & chkbit);
		w <<= 1;
		if (flg) w |= 1;
		rot--;
	}
	return (save | (w & mask));
}

DWORD __fastcall dword_rangerot_right(DWORD w, int range, int rot)
{
	DWORD chkbit = 1;
	DWORD mask = 1;
	while (--range > 0) { chkbit <<= 1; mask = (mask << 1) | 1; }

	DWORD save = w & (0xffffffff^mask);
	w &= mask;
	while (rot > 0) {
		BOOL flg = (w & 1);
		w >>= 1;
		if (flg) w |= chkbit;
		rot--;
	}
	return (save | (w & mask));
}

static BYTE get5bit(BYTE* pcb, int index)
{
	int i = index * 5;
	while (1) {
		if (i < 8) break;
		pcb++;
		i -= 8;
	}

	BYTE cb;
	switch (i) {
		case 0:
// 1111111122222222
// aaaaa
			cb = *pcb >> 3;
			break;

		case 1:
// 1111111122222222
//  aaaaa
			cb = (*pcb >> 2) & 0x1f;
			break;

		case 2:
// 1111111122222222
//   aaaaa
			cb = (*pcb >> 1) & 0x1f;
			break;

		case 3:
// 1111111122222222
//    aaaaa
			cb = *pcb & 0x1f;
			break;

		case 4:
// 1111111122222222
//     aaaaa
			cb = (*pcb << 1) & 0x1f;
			cb |= *(pcb + 1) >> 7;
			break;

		case 5:
// 1111111122222222
//      aaaaa
			cb = (*pcb << 2) & 0x1f;
			cb |= *(pcb + 1) >> 6;
			break;

		case 6:
// 1111111122222222
//       aaaaa
			cb = (*pcb << 3) & 0x1f;
			cb |= *(pcb + 1) >> 5;
			break;

		case 7:
// 1111111122222222
//        aaaaa
			cb = (*pcb << 4) & 0x1f;
			cb |= *(pcb + 1) >> 4;
			break;
	}
	return cb;
}

static void put5bit(BYTE* pcb, int index, BYTE cb)
{
	int i = index * 5;
	while (1) {
		if (i < 8) break;
		pcb++;
		i -= 8;
	}

	switch (i) {
		case 0:
// 1111111122222222
// aaaaa
			*pcb &= 0x07;
			*pcb |= (cb << 3);
			break;

		case 1:
// 1111111122222222
//  aaaaa
			*pcb &= 0x83;
			*pcb |= (cb << 2);
			break;

		case 2:
// 1111111122222222
//   aaaaa
			*pcb &= 0xc1;
			*pcb |= (cb << 1);
			break;

		case 3:
// 1111111122222222
//    aaaaa
			*pcb &= 0xe0;
			*pcb |= cb;
			break;

		case 4:
// 1111111122222222
//     aaaaa
			*pcb       &= 0xf0;
			*pcb       |= (cb >> 1);
			*(pcb + 1) &= 0x7f;
			*(pcb + 1) |= (cb << 7);
			break;

		case 5:
// 1111111122222222
//      aaaaa
			*pcb       &= 0xf8;
			*pcb       |= (cb >> 2);
			*(pcb + 1) &= 0x3f;
			*(pcb + 1) |= (cb << 6);
			break;

		case 6:
// 1111111122222222
//       aaaaa
			*pcb       &= 0xfc;
			*pcb       |= (cb >> 3);
			*(pcb + 1) &= 0x1f;
			*(pcb + 1) |= (cb << 5);
			break;

		case 7:
// 1111111122222222
//        aaaaa
			*pcb       &= 0xfe;
			*pcb       |= (cb >> 4);
			*(pcb + 1) &= 0x0f;
			*(pcb + 1) |= (cb << 4);
			break;
	}
}

////////////////////////////////////////////////////////////////
// for FileVisor5

#include "codec_fv5.cxx"

////////////////////////////////////////////////////////////////
// for Pixef2

#include "codec_pixef2.cxx"






////////////////////////////////////////////////////////////////
//

void encode_core(BYTE* map_key_data, int prx, int year, int month, int day, int seq, LPSTR pszSeq)
{
	BYTE bitkey[12];

	int index = seq % 6;
	year -= 1980;
	WORD date_1 = K2C_YEAR(year) + K2C_MON(month) + K2C_DAY(day);
	WORD date_2 = L2C_YEAR(year) + L2C_MON(month) + L2C_DAY(day);

	DWORD seq_1 = seq + date_2;
	DWORD seq_2 = dword_rangerot_right(seq_1, 17, index + 1);

	BYTE prx_1 = byte_rangerot_left((BYTE)prx, 7, index + 1);
	BYTE prx_2 = byte_rangerot_right((BYTE)prx, 7, index + 1);

	bitkey[ 2] = (BYTE)((seq_1 >> 9) & 0xff);
	bitkey[ 3] = (BYTE)((seq_1 >> 1) & 0xff);
	bitkey[ 4] = (BYTE)((seq_1 << 7) & 0x80) | (BYTE)(date_1 >> 7);
	bitkey[ 5] = (BYTE)((date_1 << 1) & 0xfe) | (prx_1 >> 6);
	bitkey[ 6] = (prx_1 << 2) | (BYTE)(((date_2) >> 12) & 0x03);
	bitkey[ 7] = (BYTE)((date_2 >> 4) & 0xff);
	bitkey[ 8] = (BYTE)((date_2 << 4) & 0xf0) | (BYTE)(prx_2 >> 3);
	bitkey[ 9] = (BYTE)((prx_2 << 5) & 0xf0) | (BYTE)((seq_2 >> 12) & 0xff);
	bitkey[10] = (BYTE)((seq_2 >> 4) & 0xff);
	bitkey[11] = (BYTE)((seq_2 << 4) & 0xf0);
	bitkey[12] = (BYTE)((index + 1) << 4);

	WORD crc = crc16(10, &bitkey[2]);
	bitkey[0] = (BYTE)(crc >> 8);
	bitkey[1] = (BYTE)(crc & 0xff);

	DWORD bfdata[2];
	bfdata[0] = *((DWORD*)bitkey);
	bfdata[1] = *((DWORD*)bitkey + 1);

	BF_KEY key;
	bf_setkey(&key, 2, (BYTE*)&bitkey[8]);
	bf_encrypt(bfdata, &key);

	*((DWORD*)bitkey) = bfdata[0];
	*((DWORD*)bitkey + 1) = bfdata[1];

	int pos = 0;
	char* p = pszSeq;
	for (int i = 0; i < 20; i++) {
		int n = get5bit(bitkey, i);
		if (i < 19)		*p++ = map_key_data[index * 32 + n] + 0x20;
		else			*p++ = '0' + n;
		pos++;
		if (!(pos % 5))	*p++ = '-';
	}
	pszSeq[23] = '\0';
}

BOOL decode_core(BYTE* map_key_data, LPCSTR pszSrc, LPSTR pszDst)
{
	char buf[MAX_PATH];
	char* p = (char*)pszSrc;
	char* q = buf;
	while (*p) {
		if (*p != '-') {
			if (*(p + 1))	*q++ = *p - 0x20;
			else			*q++ = *p;
		}
		p++;
	}
	*q = '\0';

	int index = buf[19] - '1';
	if (index < 0 || index > 5) {
		lstrcpy(pszDst, "index error");
		return FALSE;
	}

	BYTE bitkey[12];
	memset(bitkey, 0, sizeof(bitkey));
	q = buf;
	int i;
	for (i = 0; i < 20; i++) {
		if (i < 19) {
			int j;
			for (j = 0; j < 32; j++) {
				if (map_key_data[index * 32 + j] == *q) {
					put5bit(bitkey, i, j);
					break;
				}
			}
			if (j == 32) {
				lstrcpy(pszDst, "mapchar error");
				return FALSE;
			}
		}
		else
			put5bit(bitkey, i, index + 1);
		q++;
	}

	DWORD bfdata[2];
	bfdata[0] = *((DWORD*)bitkey);
	bfdata[1] = *((DWORD*)bitkey + 1);

	BF_KEY key;
	bf_setkey(&key, 2, (BYTE*)&bitkey[8]);
	bf_decrypt(bfdata, &key);

	*((DWORD*)bitkey) = bfdata[0];
	*((DWORD*)bitkey + 1) = bfdata[1];

	WORD crc = ((WORD)bitkey[0] << 8) | (WORD)bitkey[1];
	if (crc != crc16(10, &bitkey[2])) {
		lstrcpy(pszDst, "crc error");
		return FALSE;
	}

	DWORD seq_2 = (((DWORD)bitkey[9] & 0x1f) << 12) |
				   ((DWORD)bitkey[10] << 4) |
				   ((DWORD)(bitkey[11] & 0xf0) >> 4);
	seq_2 = dword_rangerot_left(seq_2, 17, index + 1);

	BYTE prx_2  = ((bitkey[8] & 0x0f) << 3) | (bitkey[9] >> 5);
	prx_2 = byte_rangerot_left(prx_2, 7, index + 1);

	WORD date_2 = (((WORD)bitkey[6] & 3) << 12) |
				   ((WORD)bitkey[7] << 4) |
				   ((WORD)bitkey[8] >> 4);
	BYTE prx_1  = ((bitkey[5] & 1) << 6) | (bitkey[6] >> 2);
	prx_1 = byte_rangerot_right(prx_1, 7, index + 1);

	WORD date_1 = (((WORD)bitkey[4] & 0x7f) << 7) | 
				  (((WORD)bitkey[5] & 0xfe) >> 1);
	DWORD seq_1 = ((DWORD)bitkey[2] << 9) |
				  ((DWORD)bitkey[3] << 1) |
				  (((DWORD)bitkey[4] & 0x80) >> 7);

	if (prx_2 != prx_1) {
		wsprintf(pszDst, "prefix error 1=%02X, 2=%02X", prx_1, prx_2);
		return FALSE;
	}

	if (seq_1 != seq_2) {
		lstrcpy(pszDst, "seq1 not eq seq2");
		return FALSE;
	}

	if (seq_1 < date_2) {
		lstrcpy(pszDst, "sequence error");
		return FALSE;
	}
	seq_1 -= date_2;

	if ((K2E_YEAR(date_1) != L2E_YEAR(date_2)) ||
		(K2E_MON(date_1) != L2E_MON(date_2)) ||
		(K2E_DAY(date_1) != L2E_DAY(date_2)))
	{
		lstrcpy(pszDst, "date match error");
		return FALSE;
	}

	int year = K2E_YEAR(date_1) + 1980;
	int mon = K2E_MON(date_1);
	int day = K2E_DAY(date_1);

	wsprintf(pszDst, "%04d/%02d/%02d: %d", year, mon, day, seq_1);
	return TRUE;
}


////////////////////////////////////////////////////////////////
// for FileVisor6

#include "codec_fv6.cxx"

////////////////////////////////////////////////////////////////
// for FileVisor7

#include "codec_fv7.cxx"

////////////////////////////////////////////////////////////////
// for RawT

#include "codec_rawt.cxx"

