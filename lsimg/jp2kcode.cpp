#include "stdafx.h"
#include "lsimg.h"
#include "common.h"

////////////////////////////////////////////////////////////////////////
//

extern "C" {
HDIB WINAPI jp2_load(LPCSTR);
HDIB WINAPI jp2_loadW(LPCWSTR);
HDIB WINAPI jp2_load_mem(HGLOBAL);
BOOL WINAPI jp2_save(LPCSTR, HDIB);
BOOL WINAPI jp2_saveW(LPCWSTR, HDIB);
BOOL WINAPI jp2_save_ex(LPCSTR pName, HANDLE hsrc, int rate, int xcb, int ycb, BOOL bNoMct, int reslevel);
BOOL WINAPI jp2_save_exW(LPCWSTR pName, HANDLE hsrc, int rate, int xcb, int ycb, BOOL bNoMct, int reslevel);
HANDLE WINAPI jp2_save_mem_ex(HANDLE hsrc, DWORD* psize, int rate, int xcb, int ycb, BOOL bNoMct, int reslevel);
}

extern "C"
HDIB WINAPI imgJp2Load(LPCTSTR pszFile)
{
#ifdef UNICODE
	return jp2_loadW(pszFile);
#else
	return jp2_load(pszFile);
#endif
}

extern "C"
HDIB WINAPI imgJp2LoadMem(HGLOBAL hmem)
{
	return jp2_load_mem(hmem);
}

extern "C"
BOOL WINAPI imgJp2Save(LPCTSTR pszFile, HDIB hdib)
{
#ifdef UNICODE
	return jp2_saveW(pszFile, hdib);
#else
	return jp2_save(pszFile, hdib);
#endif
}

extern "C"
BOOL WINAPI imgJp2SaveEx(LPCTSTR pszFile, HDIB hdib, JP2_CPARAMS* pjc)
{
#ifdef UNICODE
	return jp2_save_exW(pszFile, hdib, pjc->rate, pjc->xcb, pjc->ycb, pjc->nomct, pjc->reslevel);
#else
	return jp2_save_ex(pszFile, hdib, pjc->rate, pjc->xcb, pjc->ycb, pjc->nomct, pjc->reslevel);
#endif
}


extern "C"
HGLOBAL WINAPI imgJp2SaveMemEx(HDIB hdib, JP2_CPARAMS* pjc, DWORD* pdwSize)
{
	return jp2_save_mem_ex(hdib, pdwSize, pjc->rate, pjc->xcb, pjc->ycb, pjc->nomct, pjc->reslevel);
}
