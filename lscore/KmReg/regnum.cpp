//////////////////////////////////////////////////////////////////////
// regnum.cpp

#include <windows.h>
#include <limits.h>

///////////////////////////////////////////////////////////////
// YYYYYYYMMMMDDDDD
// 1111111111111111
// mask d=0x001f
//      m=0x01e0
//      y=0xfe00
//
#define DTE_DAY(d)		(((d) & 0x001f))
#define DTE_MON(d)		(((d) & 0x01e0) >> 5)
#define DTE_YEAR(d)		(((d) & 0xfe00) >> 9)
#define DTC_DAY(d)		(((d) & 0x001f))
#define DTC_MON(d)		(((d) << 5) & 0x01e0)
#define DTC_YEAR(d)		(((d) << 9) & 0xfe00)

///////////////////////////////////////////////////////////////
// DDDDDMMMMYYYxxxx
// 1111111111111111
// mask d=0xf800
//      m=0x0780
//      y=0x0070
//
#define K2E_DAY(d)		(((d) & 0xf800) >> 11)
#define K2E_MON(d)		(((d) & 0x0780) >> 7)
#define K2E_YEAR(d)		(((d) & 0x0070) >> 4)
#define K2C_DAY(d)		(((d) << 11) & 0xf800)
#define K2C_MON(d)		(((d) << 7) & 0x0780)
#define K2C_YEAR(d)		(((d) << 4) & 0x0070)

#define LODWORD(a)				(*((DWORD*)&(a)))
#define HIDWORD(a)				(*(((DWORD*)&(a)) + 1))
#define SETDWORDLONG(a,b,c)		(*((DWORD*)&(a))=(b),*(((DWORD*)&(a))+1)=(c))

#define CRCPOLY16		0x1021
#define CRCPOLY32		0x04C11DB7

static WORD crc16tbl[UCHAR_MAX + 1];
static DWORD crc32tbl[UCHAR_MAX + 1];
static char szEncNum[MAX_PATH];
static int dmaxtbl[12] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/////////////////////////////////////////////////////////////////////
//

void initcrc32tbl()
{
	register UINT i, j;
	register ULONG r;
	for (i = 0; i < UCHAR_MAX; i++) {
		r = (ULONG)i << (32 - CHAR_BIT);
		for (j = 0; j < CHAR_BIT; j++) {
			if (r & 0x80000000)
				r = (r << 1) ^ CRCPOLY32;
			else
				r <<= 1;
		}
		crc32tbl[i] = r & 0xFFFFFFFF;
	}
}

DWORD crc32(int n, BYTE* pcb)
{
	register DWORD r = 0xFFFFFFFF;
	while (--n >= 0)
		r = (r << CHAR_BIT) ^ crc32tbl[(BYTE)(r >> (32 - CHAR_BIT)) ^ *pcb++];
	return ~r & 0xFFFFFFFF;
}

void initcrc16tbl()
{
	register WORD i, j , r;
	for (i = 0; i <= UCHAR_MAX; i++) {
		r = i << (16 - CHAR_BIT);
		for (j = 0; j < CHAR_BIT; j++) {
			if (r & 0x8000)
				r = (r << 1) ^ CRCPOLY16;
			else
				r <<= 1;
		}
		crc16tbl[i] = r & 0xFFFF;
	}
}

WORD crc16(int n, BYTE* pcb)
{
	register WORD r = 0xFFFF;
	while (--n >= 0)
		r = (r << CHAR_BIT) ^ crc16tbl[(BYTE)(r >> (16 - CHAR_BIT)) ^ *pcb++];
	return ~r & 0xFFFF;
}

// V‚µ‚¢ƒpƒX‚Í xxxx xxxx xxxx xxxx
//                             ---- CRC
//                        ----      SEQ
//                   ---?           DATE
//              ----                PREFIX(LOBYTE=pref, HIBYTE=check)
_inline static void fv4_encnum_2000(int nPrx, int nYear, int nMonth, int nDay, int nSeq, LPSTR pszSeq)
{
	int delta;
	if (nSeq & 1)
		delta = nSeq & 0xff;
	else
		delta = (nSeq & 0xff00) >> 8;
	if (!delta)
		delta++;
	delta = ((delta & 0xf) << 4) | ((delta & 0xf0) >> 4);
	nPrx |= ((delta << 8) & 0xff00);

	int nDates = K2C_YEAR(nYear - 2000) + K2C_MON(nMonth) + K2C_DAY(nDay);
	int len = wsprintf(pszSeq, "%04X%04X%04X", nPrx, nDates, nSeq);
	WORD wCrc = crc16(len, (BYTE*)pszSeq);
	wCrc = (((wCrc & 0xff00) >> 8) | ((wCrc & 0x00ff) << 8));
	wCrc ^= LOWORD(nDates);
	wsprintf(&pszSeq[len], "%04X", wCrc);
}

extern "C"
void WINAPI KrEncNumber(int nPrx, int nYear, int nMonth, int nDay, int nSeq, LPSTR pszSeq)
{
	if (nYear >= 2000) {
		fv4_encnum_2000(nPrx, nYear, nMonth, nDay, nSeq, pszSeq);
		return;
	}

	register int delta = 0;
	if (nYear > 1998 || nSeq < 1000) {
		if (nSeq & 1)
			delta = nSeq & 0xf;
		else
			delta = -(nSeq & 0xf);
	}

	if (nSeq > 19999) {
		if (delta >= 0)
			delta += (nSeq & 0xf);
		else
			delta -= (nSeq & 0xf);
	}

	nPrx -= delta;

	int nDates = DTC_YEAR(nYear - 1997) + DTC_MON(nMonth) + DTC_DAY(nDay);
	int len = wsprintf(pszSeq, "%02X%04X%04X", nPrx, nSeq, nDates);
	WORD wCrc = crc16(len, (BYTE*)pszSeq);
	wCrc ^= LOWORD(nDates);
	wsprintf(&pszSeq[len], "%04X", wCrc);
}

extern "C"
LPCSTR WINAPI KrEncNumberEx(int nPrx, int nYear, int nMonth, int nDay, int nSeqNumber)
{
	KrEncNumber(nPrx, nYear, nMonth, nDay, nSeqNumber, szEncNum);
	return szEncNum;
}

extern "C"
void WINAPI KrEncodePass(int nPrx, int nYear, int nMonth, int nDay, int nSeqNumber, LPSTR pszSeq)
{
	if (nSeqNumber & 1)
		nPrx -= nSeqNumber & 0xf;
	else
		nPrx += nSeqNumber & 0xf;

	int nDates = DTC_YEAR(nYear - 1998) + DTC_MON(nMonth) + DTC_DAY(nDay);
	int len = wsprintf(pszSeq, "%02X%04X%04X", nPrx, nSeqNumber, nDates);
	DWORD dwCrc = crc32(len, (BYTE*)pszSeq);
	dwCrc ^= nDates;
	wsprintf(&pszSeq[len], "%08X", dwCrc);
}

extern "C"
LPCSTR WINAPI KrEncodePassEx(int nPrx, int nYear, int nMonth, int nDay, int nSeqNumber)
{
	KrEncodePass(nPrx, nYear, nMonth, nDay, nSeqNumber, szEncNum);
	return szEncNum;
}
