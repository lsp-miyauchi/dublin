// lspunr.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "common.h"
#include "unrarenv.h"

////////////////////////////////////////////////////////////
//

LPARCHIVERPROC gpfnProc = NULL;
UINT nRM_ARCEXTRACT = 0;
void (WINAPI* _fSHChangeNotify)(LONG, UINT, LPCVOID, LPCVOID);

////////////////////////////////////////////////////////////
//

class CRunning
{
public:
	CRunning() { _G.AddModuleRef(); }
	~CRunning() { _G.DecModuleRef(); }
};

////////////////////////////////////////////////////////////
//

static void iia2iiw(INDIVIDUALINFOA* pia, INDIVIDUALINFOW* piw)
{
	piw->dwOriginalSize		= pia->dwOriginalSize;
	piw->dwCompressedSize	= pia->dwCompressedSize;
	piw->dwCRC				= pia->dwCRC;
	piw->uFlag				= pia->uFlag;
	piw->uOSType			= pia->uOSType;
	piw->wRatio				= pia->wRatio;
	piw->wDate				= pia->wDate;
	piw->wTime				= pia->wTime;

	MultiByteToWideChar(CP_ACP, 0, pia->szFileName,  -1, piw->szFileName,  FNAME_MAX32 + 1);
	MultiByteToWideChar(CP_ACP, 0, pia->szAttribute, -1, piw->szAttribute, 8);
	MultiByteToWideChar(CP_ACP, 0, pia->szMode,      -1, piw->szMode,      8);
}

static void iiw2iia(INDIVIDUALINFOW* piw, INDIVIDUALINFOA* pia)
{
	pia->dwOriginalSize		= piw->dwOriginalSize;
	pia->dwCompressedSize	= piw->dwCompressedSize;
	pia->dwCRC				= piw->dwCRC;
	pia->uFlag				= piw->uFlag;
	pia->uOSType			= piw->uOSType;
	pia->wRatio				= piw->wRatio;
	pia->wDate				= piw->wDate;
	pia->wTime				= piw->wTime;

	WideCharToMultiByte(CP_ACP, 0, piw->szFileName,  -1, pia->szFileName,  FNAME_MAX32 + 1, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, piw->szAttribute, -1, pia->szAttribute, 8, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, piw->szMode,      -1, pia->szMode,      8, NULL, NULL);
}

////////////////////////////////////////////////////////////////////////
//

extern "C"
WORD WINAPI UnrarGetVersion()
{
	DWORD dw = GetModuleVersion();
	return (HIWORD(dw) * 100 + LOWORD(dw));
}

////////////////////////////////////////////////////////////
//

extern "C"
BOOL WINAPI DllGetFunctionPrefix(LPSTR pszPrefix, LONG cchMax)
{
	if (pszPrefix && cchMax > 3) {
		lstrcpyA(pszPrefix, "Unrar");
		return TRUE;
	}
	return FALSE;
}

extern "C"
BOOL WINAPI UnrarLocked(DWORD dw)
{
	_G.SetModuleLock(dw);
	return (BOOL)_G.IsModuleLocked();
}

extern "C"
int WINAPI UnrarA(const HWND hwnd, LPCSTR pszCmd, LPSTR pszOutput, const DWORD dwSize)
{
	MessageBoxA(hwnd, pszCmd, "UnrarA", MB_OK);
	if (!_G.IsModuleLocked()) return -1;

	return 1;
}

extern "C"
int WINAPI Unrar(const HWND hwnd, LPCSTR pszCmd, LPSTR pszOutput, const DWORD dwSize)
{
	return UnrarA(hwnd, pszCmd, pszOutput, dwSize);
}

extern "C"
int WINAPI UnrarW(const HWND hwnd, LPCWSTR pszCmd, LPWSTR pszOutput, const DWORD dwSize)
{
	if (!_G.IsModuleLocked()) return -1;

	CUnrarEnv ce;
	int res = ce.Execute(hwnd, pszCmd);
	if (pszOutput && dwSize > 0)
		wcsncpy(pszOutput, ce.m_strResult, dwSize);
	return res;
}

////////////////////////////////////////////////////////////
//

extern "C"
int WINAPI UnrarCheckArchiveW(LPCWSTR pszFileName, const int /*iMode*/)
{
	return ERROR_NOT_SUPPORT;
}

extern "C"
BOOL WINAPI UnrarCheckArchiveA(LPCSTR pszFileName, const int iMode)
{
	CStringW str(pszFileName);
	return UnrarCheckArchiveW(str, iMode);
}

extern "C"
int WINAPI UnrarCheckArchive(LPCSTR pszFileName, const int iMode)
{
	return UnrarCheckArchiveA(pszFileName, iMode);
}

////////////////////////////////////////////////////////////
//

extern "C"
BOOL WINAPI UnrarQueryFunctionList(const int iFunction)
{
	switch (iFunction) {
		case ISARC:
		case ISARC_GET_VERSION:
		case ISARC_GET_FILE_COUNT:
		case ISARC_QUERY_FUNCTION_LIST:
		case ISARC_OPEN_ARCHIVE:
		case ISARC_CLOSE_ARCHIVE:
		case ISARC_FIND_FIRST:
		case ISARC_FIND_NEXT:
		case ISARC_GET_FILE_NAME:
		case ISARC_GET_ORIGINAL_SIZE:
		case ISARC_GET_DATE:
		case ISARC_GET_TIME:
			return TRUE;

		case ISARC_CHECK_ARCHIVE:
		case ISARC_GET_ARC_FILE_NAME:
		case ISARC_GET_ARC_FILE_SIZE:
		case ISARC_GET_ARC_ORIGINAL_SIZE:
		case ISARC_GET_ARC_DATE:
		case ISARC_GET_ARC_TIME:
		case ISARC_GET_CURSOR_INTERVAL:
		case ISARC_SET_CURSOR_INTERVAL:
		case ISARC_GET_BACK_GROUND_MODE:
		case ISARC_SET_BACK_GROUND_MODE:
		case ISARC_GET_CURSOR_MODE:
		case ISARC_SET_CURSOR_MODE:
		case ISARC_GET_RUNNING:
		case ISARC_CONFIG_DIALOG:
		case ISARC_HOUT:
		case ISARC_STRUCTOUT:
		case ISARC_GET_ARC_FILE_INFO:
		case ISARC_EXTRACT:
		case ISARC_ADD:
		case ISARC_MOVE:
		case ISARC_DELETE:
		case ISARC_GET_ARC_COMPRESSED_SIZE:
		case ISARC_GET_ARC_RATIO:
		case ISARC_GET_ARC_OS_TYPE:
		case ISARC_GET_ARC_IS_SFX_FILE:
		case ISARC_GET_COMPRESSED_SIZE:
		case ISARC_GET_RATIO:
		case ISARC_GET_CRC:							// UnrarGetCRC
		case ISARC_GET_ATTRIBUTE:					// UnrarGetAttribute
		case ISARC_GET_OS_TYPE:						// UnrarGetOSType
		case ISARC_GET_METHOD:						// UnrarGetMethod
		case ISARC_GET_WRITE_TIME:					// UnrarGetWriteTime
		case ISARC_GET_CREATE_TIME:					// UnrarGetCreateTime
		case ISARC_GET_ACCESS_TIME:					// UnrarGetAccessTime
		default:
			break;
	}
	return FALSE;
}

extern "C"
HARC WINAPI UnrarOpenArchiveW(const HWND hwnd, LPCWSTR pszFileName, const DWORD dwMode)
{
	struct RAROpenArchiveDataEx data;
	memset(&data, 0, sizeof(data));
	data.ArcNameW = (wchar_t*)pszFileName;
	data.CmtBufSize = 256 * 64;
	char* comment = new char[256 * 64];
	data.CmtBuf = comment;
	data.OpenMode = (dwMode != 0) ? dwMode : RAR_OM_LIST;

	CRarArchive* rar = new CRarArchive();
	rar->m_harcdata = _G.RAROpenArchiveEx(&data);
	if (rar->m_harcdata == NULL || data.OpenResult != 0) {
		delete rar;
		delete [] comment;
		return NULL;
	}
	rar->set(data);
	return (HARC)rar;
}

extern "C"
HARC WINAPI UnrarOpenArchiveA(const HWND hwnd, LPCSTR pszFileName, const DWORD dwMode)
{
	CStringW name(pszFileName);
	return UnrarOpenArchiveW(hwnd, name, dwMode);
}

extern "C"
HARC WINAPI UnrarOpenArchive(const HWND hwnd, LPCSTR pszFileName, const DWORD dwMode)
{
	return UnrarOpenArchiveA(hwnd, pszFileName, dwMode);
}

////////////////////////////////////////////////////////////
//

extern "C"
int WINAPI UnrarCloseArchive(HARC harc)
{
	CRarArchive* rar = (CRarArchive*)harc;
	if (!rar) return -1;

	delete rar;
	return 0;
}

////////////////////////////////////////////////////////////
//

extern "C"
int WINAPI UnrarFindNextW(HARC harc, INDIVIDUALINFOW FAR* pidi)
{
	CRarArchive* rar = (CRarArchive*)harc;
	if (!rar) return -1;

	memset(&rar->m_curhdr, 0, sizeof(struct RARHeaderDataEx));
	char* comment = new char[256 * 64];
	rar->m_curhdr.CmtBuf = comment;
	rar->m_curhdr.CmtBufSize = 256 * 64;

	bool found = false;
	while (!found) {
		if (_G.RARReadHeaderEx(rar->m_harcdata, &rar->m_curhdr) != 0) break;

		if (rar->m_filter.Match(rar->m_curhdr.FileNameW)) {
			memset(pidi, 0, sizeof(INDIVIDUALINFOW));
			pidi->dwOriginalSize   = rar->m_curhdr.UnpSize;
			pidi->dwCompressedSize = rar->m_curhdr.PackSize;
			pidi->dwCRC            = rar->m_curhdr.FileCRC;
			pidi->wDate            = HIWORD(rar->m_curhdr.FileTime);
			pidi->wTime            = LOWORD(rar->m_curhdr.FileTime);
			pidi->uFlag            = (rar->m_curhdr.Flags & 0x04) ? 1 : 0;			// 暗号化
			if (rar->m_curhdr.Flags & 0x04)
				rar->m_curhdr.FileAttr |= _A_CRPT;
			DWORDLONG dls          = rar->m_curhdr.PackSize + (((DWORDLONG)rar->m_curhdr.PackSizeHigh) << 32);
			DWORDLONG dlt          = rar->m_curhdr.UnpSize + (((DWORDLONG)rar->m_curhdr.UnpSizeHigh) << 32);
			if (dlt != 0) {
				DWORDLONG ratio = (dls * 1000) / dlt;
				pidi->wRatio = LOWORD(LODWORD(ratio));
			}
			lstrcpynW(pidi->szFileName, rar->m_curhdr.FileNameW, FNAME_MAX32);
			AttrToString(pidi->szAttribute, rar->m_curhdr.FileAttr);
			found = true;
		}

		if (_G.RARProcessFile(rar->m_harcdata, RAR_SKIP, NULL, NULL) != 0) break;
	}
	delete [] comment;
	return (found ? 0 : -1);
}

extern "C"
int WINAPI UnrarFindNextA(HARC harc, INDIVIDUALINFOA FAR* pidi)
{
	INDIVIDUALINFOW iiw;
	int res = UnrarFindNextW(harc, &iiw);
	if (res != -1)
		iiw2iia(&iiw, pidi);
	return res;
}

extern "C"
int WINAPI UnrarFindNext(HARC harc, INDIVIDUALINFOA FAR* pidi)
{
	return UnrarFindNextA(harc, pidi);
}


////////////////////////////////////////////////////////////
//

extern "C"
int WINAPI UnrarFindFirstW(HARC harc, LPCWSTR pszFilter, INDIVIDUALINFOW FAR* pidi)
{
	CRarArchive* rar = (CRarArchive*)harc;
	if (!rar) return -1;

	rar->m_filter.Set(pszFilter);
	return UnrarFindNextW(harc, pidi);
}

extern "C"
int WINAPI UnrarFindFirstA(HARC harc, LPCSTR pszFilter, INDIVIDUALINFOA FAR* pidi)
{
	INDIVIDUALINFOW iiw;

	CStringW filter(pszFilter);
	int res = UnrarFindFirstW(harc, filter, &iiw);
	if (res != -1)
		iiw2iia(&iiw, pidi);
	return res;
}

extern "C"
int WINAPI UnrarFindFirst(HARC harc, LPCSTR pszFilter, INDIVIDUALINFOA FAR* pidi)
{
	return UnrarFindFirstA(harc, pszFilter, pidi);
}

////////////////////////////////////////////////////////////
//

extern "C"
int WINAPI UnrarGetMethodW(HARC harc, LPWSTR pszMethod, const int cchMax)
{
	int res = -1;
	return res;
}

extern "C"
int WINAPI UnrarGetMethodA(HARC harc, LPSTR pszMethod, const int cchMax)
{
	int res = -1;
	if (pszMethod && cchMax > 0) {
		WCHAR* wsz = new WCHAR[cchMax * 2];
		res = UnrarGetMethodW(harc, wsz, cchMax * 2);
		if (res != -1)
			WideCharToMultiByte(CP_ACP, 0, wsz, -1, pszMethod, cchMax, NULL, NULL);
	}
	return res;
}

extern "C"
int WINAPI UnrarGetMethod(HARC harc, LPSTR pszMethod, const int cchMax)
{
	return UnrarGetMethodA(harc, pszMethod, cchMax);
}

////////////////////////////////////////////////////////////
//

extern "C"
DWORD WINAPI UnrarGetOriginalSize(HARC harc)
{
	DWORD size = (DWORD)-1;
	return size;
}

extern "C"
BOOL WINAPI UnrarGetOriginalSizeEx(HARC harc, LONGLONG* pllSize)
{
	DWORD size = UnrarGetOriginalSize(harc);
	if (size != (DWORD)-1) {
		*pllSize = (LONGLONG)size;
		return TRUE;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////
//

extern "C"
WORD WINAPI UnrarGetDate(HARC harc)
{
	WORD dt = (WORD)-1;
	return dt;
}

////////////////////////////////////////////////////////////
//

extern "C"
WORD WINAPI UnrarGetTime(HARC harc)
{
	WORD tm = (WORD)-1;
	return tm;
}

extern "C" BOOL WINAPI UnrarGetAccessTimeEx(HARC /*harc*/, FILETIME* /*pft*/)
	{ return FALSE; }
extern "C" DWORD WINAPI UnrarGetAccessTime(HARC /*harc*/)
	{ return (DWORD)-1; }

extern "C"
BOOL WINAPI UnrarGetWriteTimeEx(HARC harc, FILETIME* pft)
{
	return FALSE;
}

extern "C"
DWORD WINAPI UnrarGetWriteTime(HARC harc)
{
	DWORD res = (DWORD)-1;
	FILETIME ft;
	if (UnrarGetWriteTimeEx(harc, &ft)) {
		SYSTEMTIME st;
		FileTimeToSystemTime(&ft, &st);

		struct tm at;
		memset(&at, 0, sizeof(at));
		at.tm_sec = st.wSecond;
		at.tm_min = st.wMinute;
		at.tm_hour = st.wHour;
		at.tm_mday = st.wDay;
		at.tm_mon = st.wMonth - 1;
		at.tm_year = st.wYear - 1900;

		res = (DWORD)mktime(&at);
	}
	return res;
}

////////////////////////////////////////////////////////////
//

extern "C"
int WINAPI UnrarGetAttribute(HARC harc)
{
	DWORD attr = (DWORD)-1;
	CRarArchive* rar = (CRarArchive*)harc;
	if (rar)
		attr = (DWORD)rar->m_curhdr.FileAttr;
	return attr;
}

extern "C" int WINAPI UnrarGetAttributes(HARC harc)
	{ return UnrarGetAttribute(harc); }

extern "C"
int WINAPI UnrarExtract(HARC /*harc*/, LPCSTR /*pszFileName*/, LPCSTR /*pszDir*/, DWORD /*dwMode*/)
{
	return ERROR_NOT_SUPPORT;
}

extern "C"
int WINAPI UnrarExtractMem(const HWND /*hwnd*/, LPCSTR /*pszCmd*/, LPBYTE /*pszBuffer*/, const DWORD /*dwSize*/, time_t* /*pTime*/, LPWORD /*pwAttr*/, LPDWORD /*pdwWriteSize*/)
{
	return ERROR_NOT_SUPPORT;
}

extern "C" int WINAPI UnrarExtractMemA(const HWND hwnd, LPCSTR pszCmd, LPBYTE pszBuffer, const DWORD dwSize, time_t* pTime, LPWORD pwAttr, LPDWORD pdwWriteSize)
	{ return UnrarExtractMem(hwnd, pszCmd, pszBuffer, dwSize, pTime, pwAttr, pdwWriteSize); }
extern "C" int WINAPI UnrarExtractMemW(const HWND /*hwnd*/, LPCWSTR /*pszCmd*/, LPBYTE /*pszBuffer*/, const DWORD /*dwSize*/, time_t* /*pTime*/, LPWORD /*pwAttr*/, LPDWORD /*pdwWriteSize*/)
	{ return ERROR_NOT_SUPPORT; }


extern "C"
BOOL WINAPI UnrarSetPriority(int /*nPriority*/)
{
	return FALSE;
}

extern "C"
BOOL WINAPI UnrarSetOwnerWindow(HWND hwnd)
{
	if (IsWindow(_G.GetOwnerWindow()) || UnrarGetRunning())
		return FALSE;
	_G.SetOwnerWindow(hwnd);
	return TRUE;
}

extern "C"
BOOL WINAPI UnrarClearOwnerWindow()
{
	if (UnrarGetRunning())
		return FALSE;
	_G.SetOwnerWindow(NULL);
	return TRUE;
}

extern "C"
BOOL WINAPI UnrarSetOwnerWindowEx(HWND hwnd, LPARCHIVERPROC lpArcProc)
{
	if (UnrarSetOwnerWindow(hwnd)) {
		gpfnProc = lpArcProc;
		return TRUE;
	}
	return FALSE;
}

extern "C"
BOOL WINAPI UnrarKillOwnerWindowEx(HWND hwnd)
{
	if (_G.GetOwnerWindow() != hwnd || !UnrarClearOwnerWindow())
		return FALSE;

	gpfnProc = NULL;
	return TRUE;
}

extern "C"
BOOL WINAPI UnrarConfigDialog(HWND hwnd, LPSTR /*pszOptions*/, int /*iMode*/)
{
	return FALSE;
}

extern "C" BOOL WINAPI UnrarConfigDialogA(HWND hwnd, LPSTR pszOptions, int iMode)
	{ return UnrarConfigDialog(hwnd, pszOptions, iMode); }
extern "C" BOOL WINAPI UnrarConfigDialogW(HWND /*hwnd*/, LPWSTR /*pszOptions*/, int /*iMode*/)
	{ return FALSE; }


////////////////////////////////////////////////////////////
//

extern "C"
DWORD WINAPI UnrarGetUncompressedTotalSizeW(LPCWSTR pszFileName)
{
	DWORD lTotal = 0;
	HARC harc = UnrarOpenArchiveW(NULL, pszFileName, 0);
	if (harc) {
		INDIVIDUALINFOW idi;
		if (UnrarFindFirstW(harc, L"*.*", &idi) != -1) {
			do {
				lTotal += idi.dwOriginalSize;
			} while (UnrarFindNextW(harc, &idi) != -1);
		}
		UnrarCloseArchive(harc);
	}
	return lTotal;
}

extern "C"
DWORD WINAPI UnrarGetUncompressedTotalSizeA(LPCSTR pszFileName)
{
	CStringW name(pszFileName);
	return UnrarGetUncompressedTotalSizeW(name);
}

extern "C"
DWORD WINAPI UnrarGetUncompressedTotalSize(LPCSTR pszFileName)
{
	return UnrarGetUncompressedTotalSizeA(pszFileName);
}

////////////////////////////////////////////////////////////
//

extern "C"
int WINAPI UnrarGetFileCount(LPCSTR pszArchive)
{
	int cnt = -1;
	HARC harc = UnrarOpenArchive(NULL, pszArchive, 0);
	if (harc) {
		UnrarCloseArchive(harc);
	}
	return cnt;
}

extern "C" BOOL WINAPI UnrarGetRunning()
	{ return _G.IsModuleRef(); }
extern "C" int WINAPI UnrarGetFileCountA(LPCSTR pszArchive)
	{ return UnrarGetFileCount(pszArchive); }
extern "C" int WINAPI UnrarGetFileCountW(LPCWSTR /*pszArchive*/)
	{ return ERROR_NOT_SUPPORT; }

