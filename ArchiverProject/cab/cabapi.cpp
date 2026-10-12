#include "stdafx.h"
#include "cab.h"
#include "filemap.h"


////////////////////////////////////////////////////////////
//

static WCHAR sszCabExt[] = L".cab";
static WCHAR* sszListHeader[] = {
	L"\x0d\x0aListing of cabinet file : %s\x0d\x0a\x0d\x0a",
	L"  Original   Date     Time    Attr  Type    Name\x0d\x0a",
	L"---------- -------- -------- ------ ------  ------------------------------\x0d\x0a",
	L"%10s %8s %8s %6s %-6s  %s\x0d\x0a",
	L"%10s %-10s                       %d files\x0d\x0a\x0d\x0a",
};

BOOL DoCabConfig(HWND hwndParent);

////////////////////////////////////////////////////////////
//

CCabFolder* __fastcall ConvertHarc(HARC harc)
{
	CCabFolder* pcf = static_cast<CCabFolder*>(harc);
	if (pcf && pcf->m_magic == CABMAGIC)
		return pcf;
	return NULL;
}

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
WORD WINAPI CabGetVersion()
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
		strcpy(pszPrefix, "Cab");
		return TRUE;
	}
	return FALSE;
}

extern "C"
BOOL WINAPI CabLocked(DWORD dw)
{
	_G.SetModuleLock(dw);
	return (_G.GetModuleLock() != 0);
}

extern "C"
int WINAPI CabA(const HWND hwnd, LPCSTR pszCmd, LPSTR pszOutput, const DWORD dwSize)
{
	if (!_G.IsModuleLocked()) return -1;

	CStringW cmd(pszCmd);
	CCabEnv ce;
	int res = ce.Execute(hwnd, cmd);
	if (pszOutput && dwSize > 0)
		WideCharToMultiByte(CP_ACP, 0, ce.m_strResult, -1, pszOutput, dwSize, NULL, NULL);
	return res;
}

extern "C"
int WINAPI Cab(const HWND hwnd, LPCSTR pszCmd, LPSTR pszOutput, const DWORD dwSize)
{
	return CabA(hwnd, pszCmd, pszOutput, dwSize);
}

extern "C"
int WINAPI CabW(const HWND hwnd, LPCWSTR pszCmd, LPWSTR pszOutput, const DWORD dwSize)
{
	if (!_G.IsModuleLocked()) return -1;

	CCabEnv ce;
	int res = ce.Execute(hwnd, pszCmd);
	if (pszOutput && dwSize > 0)
		wcsncpy(pszOutput, ce.m_strResult, dwSize);
	return res;
}

////////////////////////////////////////////////////////////
//

extern "C"
BOOL WINAPI CabCheckArchiveA(LPCSTR pszFileName, const int iMode)
{
	CStringW str(pszFileName);
	return CabCheckArchiveW(str, iMode);
}

extern "C"
int WINAPI CabCheckArchive(LPCSTR pszFileName, const int iMode)
{
	return CabCheckArchiveA(pszFileName, iMode);
}

extern "C"
int WINAPI CabCheckArchiveW(LPCWSTR pszFileName, const int /*iMode*/)
{
	CRunning running;

	if (!PathFileExists(pszFileName) || PathIsDirectory(pszFileName))
		return FALSE;

	FDICABINETINFO fdici;
	return CabGetCabinetInfoW(pszFileName, &fdici);
}

////////////////////////////////////////////////////////////
//

extern "C"
int WINAPI CabGetFileCount(LPCSTR pszArchive)
{
	int cnt = -1;
	HARC harc = CabOpenArchive(NULL, pszArchive, 0);
	if (harc) {
		CCabFolder* pcf = (CCabFolder*)harc;
		cnt = pcf->m_numItems;
		CabCloseArchive(harc);
	}
	return cnt;
}

extern "C" int WINAPI CabGetFileCountA(LPCSTR pszArchive)
	{ return CabGetFileCount(pszArchive); }
extern "C" int WINAPI CabGetFileCountW(LPCWSTR /*pszArchive*/)
	{ return ERROR_NOT_SUPPORT; }


////////////////////////////////////////////////////////////
//

extern "C"
BOOL WINAPI CabQueryFunctionList(const int iFunction)
{
	switch (iFunction)
	{
		case ISARC:
		case ISARC_GET_VERSION:
		case ISARC_CHECK_ARCHIVE:
		case ISARC_GET_FILE_COUNT:
		case ISARC_QUERY_FUNCTION_LIST:
		case ISARC_OPEN_ARCHIVE:
		case ISARC_CLOSE_ARCHIVE:
		case ISARC_FIND_FIRST:
		case ISARC_FIND_NEXT:
		case ISARC_GET_ARC_FILE_NAME:
		case ISARC_GET_ARC_FILE_SIZE:
		case ISARC_GET_ARC_ORIGINAL_SIZE:
		case ISARC_GET_ARC_DATE:
		case ISARC_GET_ARC_TIME:
		case ISARC_GET_FILE_NAME:
		case ISARC_GET_ORIGINAL_SIZE:
		case ISARC_GET_DATE:
		case ISARC_GET_TIME:
			return TRUE;

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
		case ISARC_GET_CRC:							// CabGetCRC
		case ISARC_GET_ATTRIBUTE:					// CabGetAttribute
		case ISARC_GET_OS_TYPE:						// CabGetOSType
		case ISARC_GET_METHOD:						// CabGetMethod
		case ISARC_GET_WRITE_TIME:					// CabGetWriteTime
		case ISARC_GET_CREATE_TIME:					// CabGetCreateTime
		case ISARC_GET_ACCESS_TIME:					// CabGetAccessTime
		default:
			break;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////
//

extern "C"
HARC WINAPI CabOpenArchiveA(const HWND hwnd, LPCSTR pszFileName, const DWORD dwMode)
{
	CStringW name(pszFileName);
	return CabOpenArchiveW(hwnd, name, dwMode);
}

extern "C"
HARC WINAPI CabOpenArchive(const HWND hwnd, LPCSTR pszFileName, const DWORD dwMode)
{
	return CabOpenArchiveA(hwnd, pszFileName, dwMode);
}

extern "C"
HARC WINAPI CabOpenArchiveW(const HWND hwnd, LPCWSTR pszFileName, const DWORD dwMode)
{
	CRunning running;
	return (HARC)FDIOpenCabinet(hwnd, pszFileName, dwMode);
}

////////////////////////////////////////////////////////////
//

extern "C"
int WINAPI CabCloseArchive(HARC harc)
{
	CCabFolder* pcf = ConvertHarc(harc);
	if (pcf) {
		delete pcf;
		return 0;
	}
	return -1;
}

////////////////////////////////////////////////////////////
//

extern "C"
int WINAPI CabFindFirstA(HARC harc, LPCSTR pszFilter, INDIVIDUALINFOA FAR* pidi)
{
	INDIVIDUALINFOW iiw;
	CStringW filter(pszFilter);
	int res = CabFindFirstW(harc, filter, &iiw);
	if (res != -1)
		iiw2iia(&iiw, pidi);
	return res;
}

extern "C"
int WINAPI CabFindFirst(HARC harc, LPCSTR pszFilter, INDIVIDUALINFOA FAR* pidi)
{
	return CabFindFirstA(harc, pszFilter, pidi);
}

extern "C"
int WINAPI CabFindFirstW(HARC harc, LPCWSTR pszFilter, INDIVIDUALINFOW FAR* pidi)
{
	int res = -1;
	CCabFolder* pcf = ConvertHarc(harc);
	if (pcf)
		res = pcf->FindFirst(pszFilter, pidi);
	return res;
}


////////////////////////////////////////////////////////////
//

extern "C"
int WINAPI CabFindNextW(HARC harc, INDIVIDUALINFOW FAR *pidi)
{
	int res = -1;
	CCabFolder* pcf = ConvertHarc(harc);
	if (pcf)
		res = pcf->FindNext(pidi);
	return res;
}

extern "C"
int WINAPI CabFindNextA(HARC harc, INDIVIDUALINFOA FAR* pidi)
{
	INDIVIDUALINFOW iiw;
	int res = CabFindNextW(harc, &iiw);
	if (res != -1)
		iiw2iia(&iiw, pidi);
	return res;
}

extern "C"
int WINAPI CabFindNext(HARC harc, INDIVIDUALINFOA FAR* pidi)
{
	return CabFindNextA(harc, pidi);
}


////////////////////////////////////////////////////////////
//

extern "C"
int WINAPI CabGetArcFileNameA(HARC harc, LPSTR pszName, const int cchMax)
{
	int res = -1;
	if (pszName && cchMax > 0) {
		WCHAR name[MAX_PATH * 2];
		res = CabGetArcFileNameW(harc, name, ARRAYSIZE(name));
		if (res != -1)
			WideCharToMultiByte(CP_ACP, 0, name, -1, pszName, cchMax, NULL, NULL);
	}
	return res;
}

extern "C"
int WINAPI CabGetArcFileName(HARC harc, LPSTR pszName, const int cchMax)
{
	return CabGetArcFileNameA(harc, pszName, cchMax);
}

extern "C"
int WINAPI CabGetArcFileNameW(HARC harc, LPWSTR pszName, const int cchMax)
{
	int res = -1;
	CCabFolder* pcf = ConvertHarc(harc);
	if (pcf)
		res = pcf->GetArcFileName(pszName, cchMax);
	return res;
}

////////////////////////////////////////////////////////////
//


extern "C"
DWORD WINAPI CabGetArcFileSize(HARC harc)
{
	DWORD size = (DWORD)-1;
	CCabFolder* pcf = ConvertHarc(harc);
	if (pcf) {
		LARGE_INTEGER li;
		if (pcf->GetArcFileSize(&li))
			size = li.LowPart;
	}
	return size;
}

extern "C"
BOOL WINAPI CabGetArcFileSizeEx(HARC harc, LONGLONG* pllSize)
{
	CCabFolder* pcf = ConvertHarc(harc);
	if (pllSize && pcf) {
		LARGE_INTEGER li;
		if (pcf->GetArcFileSize(&li)) {
			*pllSize = li.QuadPart;
			return TRUE;
		}
	}
	return FALSE;
}

extern "C"
DWORD WINAPI CabGetArcOriginalSize(HARC harc)
{
	DWORD size = (DWORD)-1;
	CCabFolder* pcf = ConvertHarc(harc);
	if (pcf) {
		LARGE_INTEGER li;
		if (pcf->GetArcOriginalSize(&li))
			size = li.LowPart;
	}
	return size;
}

extern "C"
BOOL WINAPI CabGetArcOriginalSizeEx(HARC harc, LONGLONG* pllSize)
{
	CCabFolder* pcf = ConvertHarc(harc);
	if (pllSize && pcf) {
		LARGE_INTEGER li;
		if (pcf->GetArcOriginalSize(&li)) {
			*pllSize = li.QuadPart;
			return TRUE;
		}
	}
	return FALSE;
}

extern "C"
WORD WINAPI CabGetArcDate(HARC harc)
{
	WORD dt = (WORD)-1;
	CCabFolder* pcf = ConvertHarc(harc);
	if (pcf)
		pcf->GetArcDosDateTime(&dt, NULL);
	return dt;
}

extern "C"
WORD WINAPI CabGetArcTime(HARC harc)
{
	WORD tm = (WORD)-1;
	CCabFolder* pcf = ConvertHarc(harc);
	if (pcf)
		pcf->GetArcDosDateTime(NULL, &tm);
	return tm;
}

static BOOL __CabGetArcTimeEx(HARC harc, WIN32_FIND_DATA& wfd)
{
	CCabFolder* pcf = ConvertHarc(harc);
	if (pcf) {
		wfd = pcf->m_wfd;
		return TRUE;
	}
	return FALSE;
}

extern "C"
BOOL WINAPI CabGetArcCreateTimeEx(HARC harc, FILETIME* pft)
{
	WIN32_FIND_DATA wfd;
	if (pft && __CabGetArcTimeEx(harc, wfd)) {
		*pft = wfd.ftCreationTime;
		return TRUE;
	}
	return FALSE;
}

extern "C"
BOOL WINAPI CabGetArcWriteTimeEx(HARC harc, FILETIME* pft)
{
	WIN32_FIND_DATA wfd;
	if (pft && __CabGetArcTimeEx(harc, wfd)) {
		*pft = wfd.ftLastWriteTime;
		return TRUE;
	}
	return FALSE;
}

extern "C"
BOOL WINAPI CabGetArcAccessTimeEx(HARC harc, FILETIME* pft)
{
	WIN32_FIND_DATA wfd;
	if (pft && __CabGetArcTimeEx(harc, wfd)) {
		*pft = wfd.ftLastAccessTime;
		return TRUE;
	}
	return FALSE;
}


////////////////////////////////////////////////////////////
//

extern "C"
int WINAPI CabGetFileNameW(HARC harc, LPWSTR pszName, const int cchMax)
{
	int res = -1;
	CCabFolder* pcf = ConvertHarc(harc);
	if (pcf)
		res = pcf->GetFileName(pszName, cchMax);
	return res;
}

extern "C"
int WINAPI CabGetFileNameA(HARC harc, LPSTR pszName, const int cchMax)
{
	int res = -1;
	if (pszName && cchMax > 0) {
		WCHAR* wsz = new WCHAR[cchMax * 2];
		res = CabGetFileNameW(harc, wsz, cchMax * 2);
		if (res != -1)
			WideCharToMultiByte(CP_ACP, 0, wsz, -1, pszName, cchMax, NULL, NULL);
	}
	return res;
}

extern "C"
int WINAPI CabGetFileName(HARC harc, LPSTR pszName, const int cchMax)
{
	return CabGetFileNameA(harc, pszName, cchMax);
}


////////////////////////////////////////////////////////////
//

extern "C"
int WINAPI CabGetMethodW(HARC harc, LPWSTR pszMethod, const int cchMax)
{
	int res = -1;
	CCabFolder* pcf = ConvertHarc(harc);
	if (pcf)
		res = pcf->GetMethod(pszMethod, cchMax);
	return res;
}

extern "C"
int WINAPI CabGetMethodA(HARC harc, LPSTR pszMethod, const int cchMax)
{
	int res = -1;
	if (pszMethod && cchMax > 0) {
		WCHAR* wsz = new WCHAR[cchMax * 2];
		res = CabGetMethodW(harc, wsz, cchMax * 2);
		if (res != -1)
			WideCharToMultiByte(CP_ACP, 0, wsz, -1, pszMethod, cchMax, NULL, NULL);
	}
	return res;
}

extern "C"
int WINAPI CabGetMethod(HARC harc, LPSTR pszMethod, const int cchMax)
{
	return CabGetMethodA(harc, pszMethod, cchMax);
}

////////////////////////////////////////////////////////////
//

extern "C"
DWORD WINAPI CabGetOriginalSize(HARC harc)
{
	DWORD size = (DWORD)-1;
	CCabFolder* pcf = ConvertHarc(harc);
	if (pcf)
		size = pcf->GetOriginalSize();
	return size;
}

extern "C"
BOOL WINAPI CabGetOriginalSizeEx(HARC harc, LONGLONG* pllSize)
{
	DWORD size = CabGetOriginalSize(harc);
	if (size != (DWORD)-1) {
		*pllSize = (LONGLONG)size;
		return TRUE;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////
//

extern "C"
WORD WINAPI CabGetDate(HARC harc)
{
	WORD dt = (WORD)-1;
	CCabFolder* pcf = ConvertHarc(harc);
	if (pcf)
		dt = pcf->GetDate();
	return dt;
}

////////////////////////////////////////////////////////////
//

extern "C"
WORD WINAPI CabGetTime(HARC harc)
{
	WORD tm = (WORD)-1;
	CCabFolder* pcf = ConvertHarc(harc);
	if (pcf)
		tm = pcf->GetTime();
	return tm;
}

extern "C" BOOL WINAPI CabGetAccessTimeEx(HARC /*harc*/, FILETIME* /*pft*/)
	{ return FALSE; }
extern "C" DWORD WINAPI CabGetAccessTime(HARC /*harc*/)
	{ return (DWORD)-1; }

extern "C"
BOOL WINAPI CabGetWriteTimeEx(HARC harc, FILETIME* pft)
{
	CCabFolder* pcf = ConvertHarc(harc);
	if (pcf)
		return pcf->GetFileTime(pft);
	return FALSE;
}

extern "C"
DWORD WINAPI CabGetWriteTime(HARC harc)
{
	DWORD res = (DWORD)-1;
	FILETIME ft;
	if (CabGetWriteTimeEx(harc, &ft)) {
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
int WINAPI CabGetAttribute(HARC harc)
{
	DWORD attr = (DWORD)-1;
	CCabFolder* pcf = ConvertHarc(harc);
	if (pcf)
		attr = pcf->GetAttributes();
	return attr;
}

extern "C" int WINAPI CabGetAttributes(HARC harc)
	{ return CabGetAttribute(harc); }

extern "C"
int WINAPI CabExtract(HARC /*harc*/, LPCSTR /*pszFileName*/, LPCSTR /*pszDir*/, DWORD /*dwMode*/)
{
	return ERROR_NOT_SUPPORT;
}

extern "C"
int WINAPI CabExtractMem(const HWND /*hwnd*/, LPCSTR /*pszCmd*/, LPBYTE /*pszBuffer*/, const DWORD /*dwSize*/, time_t* /*pTime*/, LPWORD /*pwAttr*/, LPDWORD /*pdwWriteSize*/)
{
	return ERROR_NOT_SUPPORT;
}

extern "C" int WINAPI CabExtractMemA(const HWND hwnd, LPCSTR pszCmd, LPBYTE pszBuffer, const DWORD dwSize, time_t* pTime, LPWORD pwAttr, LPDWORD pdwWriteSize)
	{ return CabExtractMem(hwnd, pszCmd, pszBuffer, dwSize, pTime, pwAttr, pdwWriteSize); }
extern "C" int WINAPI CabExtractMemW(const HWND /*hwnd*/, LPCWSTR /*pszCmd*/, LPBYTE /*pszBuffer*/, const DWORD /*dwSize*/, time_t* /*pTime*/, LPWORD /*pwAttr*/, LPDWORD /*pdwWriteSize*/)
	{ return ERROR_NOT_SUPPORT; }


extern "C"
BOOL WINAPI CabSetPriority(int /*nPriority*/)
{
	return FALSE;
}

extern "C"
BOOL WINAPI CabSetOwnerWindow(HWND hwnd)
{
	if (IsWindow(_G.GetOwnerWindow()) || CabGetRunning())
		return FALSE;
	_G.SetOwnerWindow(hwnd);
	return TRUE;
}

extern "C"
BOOL WINAPI CabClearOwnerWindow()
{
	if (CabGetRunning())
		return FALSE;
	_G.SetOwnerWindow(NULL);
	return TRUE;
}

extern "C"
BOOL WINAPI CabSetOwnerWindowEx(HWND hwnd, LPARCHIVERPROC lpArcProc)
{
	if (CabSetOwnerWindow(hwnd)) {
		gpfnProc = lpArcProc;
		return TRUE;
	}
	return FALSE;
}

extern "C"
BOOL WINAPI CabKillOwnerWindowEx(HWND hwnd)
{
	if (_G.GetOwnerWindow() != hwnd || !CabClearOwnerWindow())
		return FALSE;

	gpfnProc = NULL;
	return TRUE;
}

extern "C"
BOOL WINAPI CabConfigDialog(HWND hwnd, LPSTR /*pszOptions*/, int /*iMode*/)
{
	return DoCabConfig(hwnd);
}

extern "C" BOOL WINAPI CabConfigDialogA(HWND hwnd, LPSTR pszOptions, int iMode)
	{ return CabConfigDialog(hwnd, pszOptions, iMode); }
extern "C" BOOL WINAPI CabConfigDialogW(HWND /*hwnd*/, LPWSTR /*pszOptions*/, int /*iMode*/)
	{ return FALSE; }


////////////////////////////////////////////////////////////
//

extern "C"
BOOL WINAPI CabGetNextCabinet(HARC harc, LPSTR pszNextCab, int cchMax)
{
	BOOL bOK = FALSE;
	CCabFolder* pcf = ConvertHarc(harc);
	if (pcf)
		bOK = pcf->GetNextCabinet(pszNextCab, cchMax);
	return bOK;
}

////////////////////////////////////////////////////////////
//

extern "C"
BOOL WINAPI CabGetCabinetInfoA(LPCSTR pszCab, void* pv)
{
	CStringW str(pszCab);
	return CabGetCabinetInfoW(str, pv);
}

extern "C"
BOOL WINAPI CabGetCabinetInfo(LPCSTR pszCab, void* pv)
{
	return CabGetCabinetInfoA(pszCab, pv);
}

extern "C"
BOOL WINAPI CabGetCabinetInfoW(LPCWSTR pszCab, void* pv)
{
	return FDIGetCabinetInfo(pszCab, (FDICABINETINFO*)pv);
}

////////////////////////////////////////////////////////////
//

extern "C"
DWORD WINAPI CabGetUncompressedTotalSizeA(LPCSTR pszCab)
{
	CStringW cab(pszCab);
	return CabGetUncompressedTotalSizeW(cab);
}

extern "C"
DWORD WINAPI CabGetUncompressedTotalSize(LPCSTR pszCab)
{
	return CabGetUncompressedTotalSizeA(pszCab);
}

extern "C"
DWORD WINAPI CabGetUncompressedTotalSizeW(LPCWSTR pszCab)
{
	DWORD lTotal = 0;
	HARC harc = CabOpenArchiveW(NULL, pszCab, 0);
	if (harc) {
		INDIVIDUALINFOW idi;
		if (CabFindFirstW(harc, gszAst, &idi) != -1) {
			do {
				lTotal += idi.dwOriginalSize;
			} while (CabFindNextW(harc, &idi) != -1);
		}
		CabCloseArchive(harc);
	}
	return lTotal;
}

#ifdef _CABSFX_SUPPORT

extern "C"
BOOL WINAPI CabGetCabinetFromSfxA(LPCSTR pszSfx, LPCSTR pszPath)
{
	CStringW sfx(pszSfx);
	CStringW path(pszPath);
	return CabGetCabinetFromSfxW(sfx, path);
}

extern "C"
BOOL WINAPI CabGetCabinetFromSfx(LPCSTR pszSfx, LPCSTR pszPath)
{
	return CabGetCabinetFromSfxA(pszSfx, pszPath);
}

extern "C"
BOOL WINAPI CabGetCabinetFromSfxW(LPCWSTR pszSfx, LPCWSTR pszPath)
{
	CWaitingCursor wc;

	BOOL bOK = FALSE;
	HMODULE hmod = LoadLibraryEx(pszSfx, NULL, LOAD_LIBRARY_AS_DATAFILE);
	if (hmod) {
		HRSRC hr = FindResource(hmod, L"CABINET", RT_RCDATA);
		if (hr) {
			DWORD dwCabSize = SizeofResource(hmod, hr);
			HGLOBAL hglb = LoadResource(hmod, hr);
			if (hglb) {
				LPBYTE lpCab = (LPBYTE)LockResource(hglb);
				if (lpCab) {
					WCHAR* p = PathFindFileName(pszSfx);

					WCHAR cab[MAX_PATH * 2];
					PathCombine(cab, pszPath, p);
					PathRenameExtension(cab, sszCabExt);

					HANDLE hf = CreateFile(cab, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
					if (hf != INVALID_HANDLE_VALUE) {
						DWORD dw;
						bOK = (WriteFile(hf, lpCab, dwCabSize, &dw, NULL) && dw == dwCabSize);
						CloseHandle(hf);

						if (!bOK)
							DeleteFile(cab);
					}
				}
			}
		}
		FreeLibrary(hmod);
	}
	return bOK;
}
#else

extern "C" BOOL WINAPI CabGetCabinetFromSfxA(LPCSTR, LPCSTR) { return FALSE; }
extern "C" BOOL WINAPI CabGetCabinetFromSfx(LPCSTR, LPCSTR) { return FALSE; }
extern "C" BOOL WINAPI CabGetCabinetFromSfxW(LPCWSTR, LPCWSTR) { return FALSE; }

#endif		// _CABSFX_SUPPORT

////////////////////////////////////////////////////////////////////////
//

int CCabEnv::CmdList()
{
	INDIVIDUALINFOW idi;
	HARC harc = CabOpenArchiveW(m_hwndOwner, m_strCabName, 0);
	if (harc) {
		int nFiles = 0;
		long lTotalSize = 0;
		if (!CabFindFirstW(harc, NULL, &idi)) {
			msgout(sszListHeader[0], (LPCWSTR)m_strCabName);
			msgout(sszListHeader[1]);
			msgout(sszListHeader[2]);

			WCHAR s[32], t[32], u[32];
			do {
				swprintf(s, L"%d", idi.dwOriginalSize);
				FormatDosDate(idi.wDate, t);
				FormatDosTime(idi.wTime, u);
				msgout(sszListHeader[3],
							s,
							t,
							u,
							idi.szAttribute,
							idi.szMode,
							idi.szFileName);
				nFiles++;
				lTotalSize += idi.dwOriginalSize;
			} while (!CabFindNextW(harc, &idi));

			msgout(sszListHeader[2]);

			swprintf(s, L"%d", lTotalSize);
			StrFormatByteSize(lTotalSize, t, ARRAYSIZE(t));
			swprintf(u, L"(%s)", t);
			msgout(sszListHeader[4], s, u, nFiles);
		}
		CabCloseArchive(harc);
		return 0;
	}
	return -1;
}

int CCabEnv::CmdCheck()
{
	FDICABINETINFO fdici;
	if (CabGetCabinetInfoW(m_strCabName, &fdici)) {
		msgout( L"Information on cabinet file '%s'\n"
				L"   Total length of cabinet file : %d\n"
				L"   Number of folders in cabinet : %d\n"
				L"   Number of files in cabinet   : %d\n"
				L"   Cabinet set ID               : %d\n"
				L"   Cabinet number in set        : %d\n",
				(LPCWSTR)m_strCabName,
				fdici.cbCabinet,
				fdici.cFolders,
				fdici.cFiles,
				fdici.setID,
				fdici.iCabinet);
		return 0;
	}
	return -1;
}

int CCabEnv::CmdTest()
{
	msgout(L"testing cabinet: %s\n", (LPCWSTR)m_strCabName);
	int r = -1;

	filemap fm;
	if (fm.open(m_strCabName)) {
		register BYTE* pb = fm.m_base;
		register DWORD s = fm.m_size;
		while (s) {
			if (s < sizeof(CFHEADER))
				break;

			if (*((DWORD*)pb) == 0x4643534d &&
				*((DWORD*)(pb + 4)) == 0 &&
				*((DWORD*)(pb + 8)) != 0)
			{
				if (CabCheckInside(this, pb))
					r = 0;
				break;
			}
			pb++;
			s--;
		}
		fm.close();
	}
	else
		msgout(L" cannot open cabinet\n");
	return r;
}

BOOL CCabEnv::PerseResponseFile(LPCWSTR pFileList)
{
	CStringA str(pFileList);
	FILE* fp = fopen(str, "r");
	if (fp == NULL) return FALSE;

	WORD w;
	fread(&w, 2, 1, fp);
	if (w == 0xfeff || w == 0xfffe) {
		// UTF16 LE/BEの判定
		fclose(fp);
		return PerseResponseFileW(pFileList);
	}
	fseek(fp, 0, SEEK_SET);

	int i, len;
	char* textptr = new char[MAX_PATH * 4];
	while (fgets(textptr, MAX_PATH * 4, fp)) {
		len = (int)strlen(textptr);
		if (len > 0) {
			for (i = 0; i < len; i++) {
				if (textptr[i] == '\n' || textptr[i] == '\r') {
					textptr[i] = 0;
					break;
				}
			}
		}

		strseparatorA ssp;
		len = ssp.set(textptr, ' ');
		for (i = 0; i < len; i++) {
			CStringW wstr(ssp[i]);
			wstr.Replace('/', '\\');
			m_arrMask.Add(wstr.Trim(L' '));
		}
	}
	delete [] textptr;
	fclose(fp);
	return TRUE;
}

BOOL CCabEnv::PerseResponseFileW(LPCWSTR pFileList)
{
	FILE* fp = _wfopen(pFileList, L"rb");
	if (fp == NULL) return FALSE;

	BYTE cb[2];
	if (fread(cb, 1, 2, fp) != 2) return FALSE;

	// UTF16 LE/BEの判定 指定無し = 16LE
	bool b16le = true;
	if (cb[0] == 0xfe && cb[1] == 0xff)
		b16le = false;
	else if (cb[0] != 0xff || cb[1] != 0xfe)
		fseek(fp, 0, SEEK_SET);

	int i, len;
	WCHAR* textptr = new WCHAR[MAX_PATH * 4];
	while (fgetws(textptr, MAX_PATH * 4, fp)) {
		len = (int)wcslen(textptr);

		if (len > 0) {
			for (i = 0; i < len; i++) {
				if (!b16le) {
					WCHAR ch = textptr[i];
					textptr[i] = (((ch >> 8) & 0xff) | ((ch & 0xff) << 8));
				}

				if (textptr[i] == '\n' || textptr[i] == '\r') {
					textptr[i] = 0;
					break;
				}
			}
		}

		strseparatorW ssp;
		len = ssp.set(textptr, ' ');
		for (i = 0; i < len; i++) {
			CStringW wstr = ssp[i];
			wstr.Replace('/', '\\');
			m_arrMask.Add(wstr.Trim(L' '));
		}
	}
	delete [] textptr;
	fclose(fp);
	return TRUE;
}

BOOL CCabEnv::PerseSwitch(LPCWSTR pszSwitch)
{
	WCHAR ch, buf[MAX_PATH * 2];
	int n;

	LPCWSTR p = pszSwitch;
	while (*p) {
		switch (*p) {
		case 'g':		// System/Hiddenファイルを格納しない
			m_cmdOption |= CABOPT_EXC_SYSHIDDEN;
			break;
		case 'h':		// 作業フォルダ
			if (*++p) {
				wcscpy(buf, p);
				PathUnquoteSpacesW(buf);
				m_strWorkPath = buf;
				return TRUE;
			}
			return FALSE;
		case 'i':		// サイレントモード
			m_cmdOption |= CABOPT_SILENT;
			break;
		case 'j':		// フォルダを無視して解凍または圧縮
			m_cmdOption |= CABOPT_IGNOREDIR;
			break;
		case 'k':		// 連続するキャビネットを解凍
			m_cmdOption |= CABOPT_KEEPNEXT;
			break;
		case 'm':		// 圧縮方式
			if (*++p) {
				if (*p == 'l') {			// LZX圧縮
					m_cmdOption |= CABOPT_LZX;
					m_nCompressionLevel = 21;
				}
				else if (*p == 'z')			// MSZIP圧縮
					m_cmdOption |= CABOPT_MSZIP;
				else if (*p == 'q') {		// QUANTUM圧縮
					m_cmdOption |= CABOPT_QUANTUM;
					m_nCompressionLevel = 7;
					m_nMemoryLevel = 21;
				}
				else if (*p == 's')			// 無圧縮
					m_cmdOption |= CABOPT_STORE;
				else
					return FALSE;

				if (*++p == ':') {
					n = StrToInt(++p);
					if (m_cmdOption | CABOPT_LZX) {
						m_nCompressionLevel = min(21, max(15, n));
					}
					else if (m_cmdOption | CABOPT_QUANTUM) {
						n = min(7, max(1, n));
						m_nCompressionLevel = n;

						// メモリレベルの取得
						if (*p == ',' || *p == '/' || *p == '.') {
							n = StrToInt(++p);
							m_nMemoryLevel = min(21, max(10, n));
						}
					}
				}
			}
			return TRUE;
		case 'n':		// 新しいファイルのみ
			m_cmdOption |= CABOPT_NEWONLY;
			break;
		case 'o':		// 無条件に上書き
			m_cmdOption |= CABOPT_OVERWRITE;
			break;
		case 'p':		// フルパスで一致させる
			m_cmdOption |= CABOPT_FULLPATHMATCH;
			break;
		case 'q':		// 8+3形式で処理
			m_cmdOption |= CABOPT_DOSNAME;
			break;
		case 'r':		// ディレクトリを再帰で
			m_cmdOption |= CABOPT_RECURSEDIR;
			break;
		case 's':		// セットID
			if (*(p + 1) == ':') {
				p += 2;
				m_sSetID = (short)StrToInt(p);
				return TRUE;
			}
			return FALSE;
		case 'v':		// マルチボリューム
			if (*(p + 1) == ':') {
				m_cmdOption |= CABOPT_MULTIVOL;
				p += 2;
				m_nVolumeSize = StrToInt(p) * 1024;
				if (m_nVolumeSize <= 0)
					m_nVolumeSize = CB_MAX_DISK_NEW;
			}
			else
				m_nVolumeSize = CB_MAX_DISK_NEW;
			return TRUE;
		case 'u':		// UTFエンコード
			if (!wcscmp(p, L"utf")) {
				m_cmdOption |= CABOPT_UTFENCODE;
				return TRUE;
			}
			return FALSE;
		case 'w':		// 正規表現を利用
			m_cmdOption |= CABOPT_REGEXP;
			break;
		case 'y':		// 日時指定
			ch = *++p;
			if (ch != 's' && ch != 'e')
				return FALSE;
			n = (int)wcslen(++p);
			if (n < 8 || n & 1)
				return FALSE;

			SYSTEMTIME st;
			memset(&st, 0, sizeof(st));
			n = swscanf(p, L"%4hd%2hd%2hd%2hd%2hd%2hd",
							&st.wYear, &st.wMonth, &st.wDay,
							&st.wHour, &st.wMinute, &st.wSecond);
			if (n == EOF || n < 3)
				return FALSE;

			switch (n) {
			case 3:	st.wHour         = (ch == 's') ? 0 : 23;
			case 4:	st.wMinute       = (ch == 's') ? 0 : 59;
			case 5: st.wSecond       = (ch == 's') ? 0 : 59;
			case 6: st.wMilliseconds = (ch == 's') ? 0 : 999;
			default:	break;
			}

			if (ch == 's') {
				m_cmdOption |= CABOPT_DTSTART;
				SystemTimeToFileTime(&st, &m_ftStart);
			}
			else {
				m_cmdOption |= CABOPT_DTEND;
				SystemTimeToFileTime(&st, &m_ftEnd);
			}
			return TRUE;
		case 'z':
			++p;
			if (*p == 's')
				m_strStored = ++p;						//  zs: 無圧縮格納するファイルのフィルタ指定
			else if (*p == 'e')
				m_strExcludes = ++p;					//  ze: 格納しないファイルのフィルタ指定
			else
				return FALSE;
			return TRUE;
		case '~':		// デバッグオプション
			m_cmdOption |= CABOPT_DEBUG;
			break;
		default:
			return FALSE;
		}
		p++;
	}
	return TRUE;
}

BOOL CCabEnv::PerseCommand(LPCWSTR pszCmd)
{
	WCHAR buf[MAX_PATH];
	GetCurrentDirectory(ARRAYSIZE(buf), buf);
	m_strDefaultPath = buf;

	CStringW str = pszCmd;
	str.Replace('/', '\\');

	strseparator ssp;
	int cnt = ssp.set(str, ' ');
	if (cnt < 1) {
		msgout(L"invalid command line\n");
		return FALSE;
	}

	int index = 0;
	const WCHAR* p = ssp[index];

	// 命令を特定する
	if (*p == '-')
		*p++;
	if (wcslen(p) == 1) {
		switch (*p) {
		case 'a':				// キャビネット作成
		case 'm':				// キャビネット作成（移動）
		case 'x':				// キャビネット解凍
		case 'l':				// キャビネット格納一覧
		case 'c':				// キャビネットチェック
		case 't':				// キャビネットテスト（厳密チェック）
		case 'f':				// 自己解凍形式書庫作成
		case 'b':				// 自己解凍形式書庫からキャビネットを抽出
			m_chCmd = *p;
			break;
		default:
			msgout(L"invalid command '%c'\n", *p);
			return FALSE;
		}
		++index;
	}
	else
		m_chCmd = CABCMD_LIST;

	m_cmdOption = 0;
	BOOL bOK;
	for (; index < cnt; index++) {
		p = ssp[index];
		if (*p == '-') {
			if (!PerseSwitch(++p)) {
				msgout(L"invalid switch '%s'\n", p);
				return FALSE;
			}
		}
		else if (*p == '@') {		// ファイルリスト
			p++;
			if (*p == '@') {
				// "@@<respfile>" … UNICODEファイル
				p++;
				bOK = PerseResponseFileW(p);
			}
			else {
				// "@<respfile>" … ASCII(SJIS)ファイル
				bOK = PerseResponseFile(p);
			}

			if (!bOK) {
				msgout(L"invalid response file '%s'\n", p);
				return FALSE;
			}
		}
		else {
			LPWSTR q = CharPrevW(p, p + wcslen(p));
			if (*q == '\\') {
				// 基準／ターゲットフォルダ
				m_strBasePath = p;
			}
			else {
				if (m_strCabName.IsEmpty()) {
					// 最初に現れたものが書庫ファイル
					m_strCabName = p;
				}
				else {
					// 以降はフィルタ または 格納ファイル名
					m_arrMask.Add(p);
					if (wcschr(p, '*') || wcschr(p, '?'))
						m_cmdOption |= CABOPT_FILTERPRESENTS;
				}
			}
		}
	}

	// キャビネットファイルが指定されていなければエラー
	if (m_strCabName.IsEmpty()) {
		msgout(L"cabinet file not specified.\n");
		return FALSE;
	}

	if (m_chCmd == CABCMD_UNPACK) {
		if (m_strBasePath.IsEmpty())
			m_strBasePath = L".\\";
	}
	return TRUE;
}

int CCabEnv::Execute(HWND hwnd, LPCWSTR pszCmd)
{
	m_hwndOwner = hwnd;
	m_strResult.Empty();
	if (!PerseCommand(pszCmd))
		return -1;

	// 書庫がパス未指定の場合
	if (m_strCabName[0] != '\\' && m_strCabName[1] != ':') {
		// 現在のフォルダを付加してフルパス名とする
		WCHAR buf[MAX_PATH * 2];
		PathCombine(buf, m_strDefaultPath, m_strCabName);
		m_strCabName = buf;
	}

	// 書庫作成の際に拡張子が指定されていなければ ".cab" を付加
	if (m_chCmd == CABCMD_PACK || m_chCmd == CABCMD_MOVE || m_chCmd == CABCMD_SFX) {
		WCHAR* p = PathFindExtension(m_strCabName);
		if (*p != '.' || _wcsicmp(p, sszCabExt))
			m_strCabName += sszCabExt;
	}

	switch (m_chCmd) {
	case CABCMD_UNPACK:
		return (FdiExtract() == TRUE) ? 0 : -1;
	case CABCMD_PACK:
	case CABCMD_MOVE:
		if (!(m_cmdOption & CABOPT_MASK_COMPRESS)) {
			switch (m_nCompressionMode) {
			case CAB_COMPRESSION_LZX:		m_cmdOption |= CABOPT_LZX; break;
			case CAB_COMPRESSION_MSZIP:		m_cmdOption |= CABOPT_MSZIP; break;
			case CAB_COMPRESSION_QUANTUM:	m_cmdOption |= CABOPT_QUANTUM; break;
			default:						m_cmdOption |= CABOPT_STORE; break;
			}
		}
		return (FciCompress() == TRUE) ? 0 : -1;
	case CABCMD_LIST:
		return CmdList();
	case CABCMD_CHECK:
		return CmdCheck();
	case CABCMD_TEST:
		return CmdTest();

#ifdef _CABSFX_SUPPORT
	case CABCMD_SFX:
		return CmdMakeSfx();
	case CABCMD_FROMSFX:
		return CabGetCabinetFromSfxW(m_strCabName, m_strBasePath) ? 0 : -1;
#endif

	default:
		break;
	}
	return -1;
}

////////////////////////////////////////////////////////////
//

extern "C" BOOL WINAPI CabGetRunning()
	{ return _G.IsModuleRef(); }
extern "C" BOOL WINAPI CabGetBackGroundMode()
	{ return FALSE; }
extern "C" BOOL WINAPI CabSetBackGroundMode(BOOL /*bBkgndMode*/)
	{ return FALSE; }
extern "C" BOOL WINAPI CabGetCursorMode()
	{ return TRUE; }
extern "C" BOOL WINAPI CabSetCursorMode(BOOL /*bCursorMode*/)
	{ return FALSE; }
extern "C" WORD WINAPI CabGetCursorInterval()
	{ return 0; }
extern "C" BOOL WINAPI CabSetCursorInterval(WORD /*wInterval*/)
	{ return TRUE; }
extern "C" DWORD WINAPI CabGetCompressedSize(HARC /*harc*/)
	{ return 0; }
extern "C" BOOL WINAPI CabGetCompressedSizeEx(HARC /*harc*/, LONGLONG* /*pllSize*/)
	{ return FALSE; }
extern "C" WORD WINAPI CabGetRatio(HARC /*harc*/)
	{ return 0; }
extern "C" DWORD WINAPI CabGetCreateTime(HARC /*harc*/)
	{ return (DWORD)-1; }
extern "C" BOOL WINAPI CabGetCreateTimeEx(HARC /*harc*/, FILETIME* /*pft*/)
	{ return FALSE; }
extern "C" DWORD WINAPI CabGetCRC(HARC /*harc*/)
	{ return (DWORD)-1; }
extern "C" UINT WINAPI CabGetOSType(HARC /*harc*/)
	{ return 0; }
extern "C" DWORD WINAPI CabGetArcCompressedSize(HARC /*harc*/)
	{ return (DWORD)-1; }
extern "C" BOOL WINAPI CabGetArcCompressedSizeEx(HARC /*harc*/, LONGLONG* /*pllSize*/)
	{ return FALSE; }
extern "C" WORD WINAPI CabGetArcRatio(HARC /*harc*/)
	{ return (WORD)-1; }
extern "C" UINT WINAPI CabGetArcOSType(HARC /*harc*/)
	{ return (UINT)-1; }
//extern "C" int WINAPI CabGetArcFileInfo(LPSTR pszArchive, MAININFO* lpMainInfo)
//	{ return -1; }
extern "C" int WINAPI CabIsSFXFile(HARC /*harc*/)
	{ return -1; }
extern "C" DWORD WINAPI CabGetArcReadSize(HARC /*harc*/)
	{ return (DWORD)-1; }
extern "C" BOOL WINAPI CabGetArcReadSizeEx(HARC /*harc*/, LONGLONG* /*pllSize*/)
	{ return FALSE; }
extern "C" int WINAPI CabCompressMem(HWND /*hwnd*/, LPCSTR /*pszCmd*/, const BYTE* /*pszBuffer*/,
									DWORD /*dwSize*/, const time_t* /*pTime*/,
									const WORD* /*pwAttributes*/, DWORD* /*pdwWriteSize*/)
	{ return ERROR_NOT_SUPPORT; }
extern "C" int WINAPI CabCompressMemA(HWND hwnd, LPCSTR pszCmd, const BYTE* pszBuffer,
									DWORD dwSize, const time_t* pTime,
									const WORD* pwAttributes, DWORD* pdwWriteSize)
	{ return  CabCompressMem(hwnd, pszCmd, pszBuffer, dwSize, pTime, pwAttributes, pdwWriteSize); }
extern "C" int WINAPI CabCompressMemW(HWND /*hwnd*/, LPCWSTR /*pszCmd*/, const BYTE* /*pszBuffer*/,
									const DWORD /*dwSize*/, const time_t* /*pTime*/,
									const WORD* /*pwAttributes*/, DWORD* /*pdwWriteSize*/)
	{ return ERROR_NOT_SUPPORT; }


