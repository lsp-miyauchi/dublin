// lspnotimpl.cpp
//

#include "stdafx.h"
#include "common.h"

////////////////////////////////////////////////////////////
//

extern "C" BOOL WINAPI UnrarGetBackGroundMode()
	{ return FALSE; }
extern "C" BOOL WINAPI UnrarSetBackGroundMode(BOOL /*bBkgndMode*/)
	{ return FALSE; }
extern "C" BOOL WINAPI UnrarGetCursorMode()
	{ return TRUE; }
extern "C" BOOL WINAPI UnrarSetCursorMode(BOOL /*bCursorMode*/)
	{ return FALSE; }
extern "C" WORD WINAPI UnrarGetCursorInterval()
	{ return 0; }
extern "C" BOOL WINAPI UnrarSetCursorInterval(WORD /*wInterval*/)
	{ return TRUE; }
extern "C" DWORD WINAPI UnrarGetCompressedSize(HARC /*harc*/)
	{ return 0; }
extern "C" BOOL WINAPI UnrarGetCompressedSizeEx(HARC /*harc*/, LONGLONG* /*pllSize*/)
	{ return FALSE; }
extern "C" WORD WINAPI UnrarGetRatio(HARC /*harc*/)
	{ return 0; }
extern "C" DWORD WINAPI UnrarGetCreateTime(HARC /*harc*/)
	{ return (DWORD)-1; }
extern "C" BOOL WINAPI UnrarGetCreateTimeEx(HARC /*harc*/, FILETIME* /*pft*/)
	{ return FALSE; }
extern "C" DWORD WINAPI UnrarGetCRC(HARC /*harc*/)
	{ return (DWORD)-1; }
extern "C" UINT WINAPI UnrarGetOSType(HARC /*harc*/)
	{ return 0; }
extern "C" DWORD WINAPI UnrarGetArcCompressedSize(HARC /*harc*/)
	{ return (DWORD)-1; }
extern "C" BOOL WINAPI UnrarGetArcCompressedSizeEx(HARC /*harc*/, LONGLONG* /*pllSize*/)
	{ return FALSE; }
extern "C" WORD WINAPI UnrarGetArcRatio(HARC /*harc*/)
	{ return (WORD)-1; }
extern "C" UINT WINAPI UnrarGetArcOSType(HARC /*harc*/)
	{ return (UINT)-1; }
//extern "C" int WINAPI UnrarGetArcFileInfo(LPSTR pszArchive, MAININFO* lpMainInfo)
//	{ return -1; }
extern "C" int WINAPI UnrarIsSFXFile(HARC /*harc*/)
	{ return -1; }
extern "C" DWORD WINAPI UnrarGetArcReadSize(HARC /*harc*/)
	{ return (DWORD)-1; }
extern "C" BOOL WINAPI UnrarGetArcReadSizeEx(HARC /*harc*/, LONGLONG* /*pllSize*/)
	{ return FALSE; }
extern "C" int WINAPI UnrarCompressMem(HWND, LPCSTR, const BYTE*, DWORD, const time_t*, const WORD*, DWORD*)
	{ return ERROR_NOT_SUPPORT; }
extern "C" int WINAPI UnrarCompressMemW(HWND, LPCWSTR, const BYTE*, const DWORD, const time_t*, const WORD*, DWORD*)
	{ return ERROR_NOT_SUPPORT; }
extern "C" int WINAPI UnrarCompressMemA(HWND, LPCSTR, const BYTE*, DWORD, const time_t*, const WORD*, DWORD*)
	{ return ERROR_NOT_SUPPORT; }
extern "C" int WINAPI UnrarGetArcFileNameW(HARC harc, LPWSTR pszName, const int cchMax)
	{ return ERROR_NOT_SUPPORT; }
extern "C" int WINAPI UnrarGetArcFileNameA(HARC harc, LPSTR pszName, const int cchMax)
	{ return ERROR_NOT_SUPPORT; }
extern "C" int WINAPI UnrarGetArcFileName(HARC harc, LPSTR pszName, const int cchMax)
	{ return ERROR_NOT_SUPPORT; }
extern "C" DWORD WINAPI UnrarGetArcFileSize(HARC harc)
	{ return ERROR_NOT_SUPPORT; }
extern "C" BOOL WINAPI UnrarGetArcFileSizeEx(HARC harc, LONGLONG* pllSize)
	{ return FALSE; }
extern "C" DWORD WINAPI UnrarGetArcOriginalSize(HARC harc)
	{ return ERROR_NOT_SUPPORT; }
extern "C" BOOL WINAPI UnrarGetArcOriginalSizeEx(HARC harc, LONGLONG* pllSize)
	{ return FALSE; }
extern "C" WORD WINAPI UnrarGetArcDate(HARC harc)
	{ return 0; }
extern "C" WORD WINAPI UnrarGetArcTime(HARC harc)
	{ return 0; }
static BOOL __GetArcTimeEx(HARC harc, WIN32_FIND_DATA& wfd)
	{ return FALSE; }
extern "C" BOOL WINAPI UnrarGetArcCreateTimeEx(HARC harc, FILETIME* pft)
	{ return FALSE; }
extern "C" BOOL WINAPI UnrarGetArcWriteTimeEx(HARC harc, FILETIME* pft)
	{ return FALSE; }
extern "C" BOOL WINAPI UnrarGetArcAccessTimeEx(HARC harc, FILETIME* pft)
	{ return FALSE; }
extern "C" int WINAPI UnrarGetFileNameW(HARC harc, LPWSTR pszName, const int cchMax)
	{ return -1; }
extern "C" int WINAPI UnrarGetFileNameA(HARC harc, LPSTR pszName, const int cchMax)
	{ return -1; }
extern "C" int WINAPI UnrarGetFileName(HARC harc, LPSTR pszName, const int cchMax)
	{ return -1; }

