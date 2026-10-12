////////////////////////////////////////////////////////////////
// for FileVisor7

#include "fv7_regkeymap.h"							// regkeymap for fv6

extern "C"
void WINAPI KrFV7(int prx, int year, int month, int day, int seq, LPSTR pszSeq)
{
	encode_core(mapbyte_fv7_regkeymap, prx, year, month, day, seq, pszSeq);
}

extern "C"
LPCSTR WINAPI KrFV7Ex(int nPrx, int nYear, int nMonth, int nDay, int nSeqNumber)
{
	encode_core(mapbyte_fv7_regkeymap, nPrx, nYear, nMonth, nDay, nSeqNumber, sszEncBuff);
	return sszEncBuff;
}

extern "C"
BOOL WINAPI KrFV7_Decode(LPCSTR pszSrc, LPSTR pszDst)
{
	return decode_core(mapbyte_fv7_regkeymap, pszSrc, pszDst);
}

