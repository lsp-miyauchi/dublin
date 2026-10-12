////////////////////////////////////////////////////////////////
// for DCRPrev

#include "dcrprev_regkeymap.h"						// regkeymap for dcrprev

extern "C"
void WINAPI KrDCRP(int prx, int year, int month, int day, int seq, LPSTR pszSeq)
{
	encode_core(mapbyte_dcrprev_regkeymap, prx, year, month, day, seq, pszSeq);
}

extern "C"
LPCSTR WINAPI KrDCRPEx(int nPrx, int nYear, int nMonth, int nDay, int nSeqNumber)
{
	encode_core(mapbyte_dcrprev_regkeymap, nPrx, nYear, nMonth, nDay, nSeqNumber, sszEncBuff);
	return sszEncBuff;
}

extern "C"
BOOL WINAPI KrDCRP_Decode(LPCSTR pszSrc, LPSTR pszDst)
{
	return decode_core(mapbyte_dcrprev_regkeymap, pszSrc, pszDst);
}
