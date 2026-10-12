#include "stdafx.h"
#include "crctable.h"

extern "C"
DWORD WINAPI lccCirRed_ul(DWORD crc, BYTE* pData, int len)
{
	CCrc32 cc;
	return cc.Calc(crc, pData, len);
}

extern "C"
DWORD WINAPI lccCalcCrc32(BYTE* pData, int len)
{
	return lccCirRed_ul(0, pData, len);
}

extern "C"
WORD WINAPI lccCirRed_us(WORD crc, BYTE* pData, int len)
{
	CCrc16 cc;
	return cc.Calc(crc, pData, len);
}

extern "C"
WORD WINAPI lccCalcCrc16(BYTE* pData, int len)
{
	return lccCirRed_us(0, pData, len);
}
