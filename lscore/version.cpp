#include "stdafx.h"
#include "lscore.h"

// winver.hで定義されている
// typedef struct tagVS_FIXEDFILEINFO
// {
//     DWORD   dwSignature;            /* e.g. 0xfeef04bd */
//     DWORD   dwStrucVersion;         /* e.g. 0x00000042 = "0.42" */
//     DWORD   dwFileVersionMS;        /* e.g. 0x00030075 = "3.75" */
//     DWORD   dwFileVersionLS;        /* e.g. 0x00000031 = "0.31" */
//     DWORD   dwProductVersionMS;     /* e.g. 0x00030010 = "3.10" */
//     DWORD   dwProductVersionLS;     /* e.g. 0x00000031 = "0.31" */
//     DWORD   dwFileFlagsMask;        /* = 0x3F for version "0.42" */
//     DWORD   dwFileFlags;            /* e.g. VFF_DEBUG | VFF_PRERELEASE */
//     DWORD   dwFileOS;               /* e.g. VOS_DOS_WINDOWS16 */
//     DWORD   dwFileType;             /* e.g. VFT_DRIVER */
//     DWORD   dwFileSubtype;          /* e.g. VFT2_DRV_KEYBOARD */
//     DWORD   dwFileDateMS;           /* e.g. 0 */
//     DWORD   dwFileDateLS;           /* e.g. 0 */
// } VS_FIXEDFILEINFO;


///////////////////////////////////////////////////////////////////
//

#pragma pack(1)
typedef struct tagVS_VERSION {
    WORD  wLength;
    WORD  wValueLength;
} VS_VERSION;
//    WORD  bText;
//    WCHAR szKey[];
//    BYTE  Padding1[];
//    VS_FIXEDFILEINFO Value;
//    BYTE  Padding2[];
//    BYTE  Children[];
//} VS_VERSION;
#pragma pack()

typedef struct tagVER_INFO {
	DWORD ressize;
	VS_VERSION* pVersionResource;
	DWORD dwTranslation;
	VS_FIXEDFILEINFO* pSignature;
} VER_INFO;

typedef struct tagCPSZ {
	int val;
	LPCTSTR psz[2];
} CPSZ;


///////////////////////////////////////////////////////////////////
//
static const TCHAR sszUnknown[] = _T("Unknown");
static const TCHAR sszUnknownJP[] = _T("不明");
static const TCHAR sszDos[] = _T("DOS");
static const TCHAR sszOS2_16[] = _T("OS/2(16)");
static const TCHAR sszOS2_32[] = _T("OS/2(32)");
static const TCHAR sszNT[] = _T("NT");
static const TCHAR sszWin16[] = _T("Windows(16)");
static const TCHAR sszWin32[] = _T("Windows(32)");
static const TCHAR sszWce[] = _T("WinCE");
static const TCHAR sszPM16[] = _T("PM(16)");
static const TCHAR sszPM32[] = _T("PM(32)");

static TCHAR* szValue[] = {
	_T("Comments"),
	_T("CompanyName"),
	_T("FileDescription"),
	_T("FileVersion"),
	_T("InternalName"),
	_T("LegalCopyright"),
	_T("LegalTrademarks"),
	_T("OriginalFilename"),
	_T("PrivateBuild"),
	_T("ProductName"),
	_T("ProductVersion"),
	_T("SpecialBuild")
};
static CPSZ cpOSType[] = {
	0,			sszUnknown,				sszUnknownJP,
	1,			sszDos,					sszDos,
	2,			sszOS2_16,				sszOS2_16,
	3,			sszOS2_32,				sszOS2_32,
	4,			sszNT,					sszNT,
	5,			sszWce,					sszWce,
	INT_MAX,	NULL,					NULL,
};
static CPSZ cpOSBase[] = {
	0,			sszUnknown,				sszUnknownJP,
	1,			sszWin16,				sszWin16,
	2,			sszPM16,				sszPM16,
	3,			sszPM32,				sszPM32,
	4,			sszWin32,				sszWin32,
	INT_MAX,	NULL,					NULL,
};
static CPSZ cpFileType[] = {
	0,			sszUnknown,				sszUnknownJP,
	1,			_T("Application"),		_T("アプリケーション"),
	2,			_T("DLL"),				_T("DLL"),
	3,			_T("Driver"),			_T("デバイスドライバ"),
	4,			_T("Font"),				_T("フォント"),
	5,			_T("VxD"),				_T("仮想デバイス"),
	7,			_T("StaticLIB"),		_T("スタティックライブラリ"),
	INT_MAX,	NULL,					NULL,
};
// VS_VERSION.dwFileSubtype for VFT_WINDOWS_DRV
static CPSZ cpFileSubTypeDrv[] = {
	0,			sszUnknown,				sszUnknown,
	1,			_T("Printer"),			_T("プリンタ"),
	2,			_T("Keyboard"),			_T("キーボード"),
	3,			_T("Language"),			_T("言語"),
	4,			_T("Display"),			_T("ディスプレイ"),
	5,			_T("Mouse"),			_T("マウス"),
	6,			_T("Network"),			_T("ネットワーク"),
	7,			_T("System"),			_T("システム"),
	8,			_T("Installable"),		_T("Installable"),
	9,			_T("Sound"),			_T("サウンド"),
	10,			_T("Comm"),				_T("通信"),
	11,			_T("InputMethod"),		_T("入力方式"),
	12,			_T("Pinter"),			_T("プリンタ"),
	INT_MAX,	NULL,					NULL,
};
// VS_VERSION.dwFileSubtype for VFT_WINDOWS_FONT
static CPSZ cpFileSubTypeFont[] = {
	0,			sszUnknown,				sszUnknown,
	1,			_T("Raster"),			_T("ラスタ"),
	2,			_T("Vector"),			_T("ベクタ"),
	3,			_T("TrueType"),			_T("TrueType"),
	INT_MAX,	NULL,					NULL,
};

///////////////////////////////////////////////////////////////////
//

static LPCTSTR cps_lookup(const CPSZ* pcp, int val)
{
	int index = (GetOEMCP() == 932) ? 1 : 0;
	for (int i = 0; ;i++) {
		if (pcp->val == INT_MAX) break;
		if (pcp->val == val) return pcp->psz[index];
		pcp++;
	}
	return (index ? sszUnknownJP : sszUnknown);
}


///////////////////////////////////////////////////////////////////
//

static VS_FIXEDFILEINFO* FindSignature(VS_VERSION* pvs, DWORD size)
{
	DWORD* ptr = (DWORD*)pvs;
	int entries = pvs->wLength / sizeof(DWORD);
	for (int i = 0; i < entries; i++) {
		if ((BYTE*)ptr >= (BYTE*)pvs + size) break;
		if (*ptr == VS_FFI_SIGNATURE) return (VS_FIXEDFILEINFO*)ptr;
		ptr++;
	}
	return NULL;
}


///////////////////////////////////////////////////////////////////
//

extern "C"
HANDLE WINAPI lccVerOpen(LPCTSTR pszName)
{
	DWORD dwHandle;
	DWORD infosize = GetFileVersionInfoSize((LPTSTR)pszName, &dwHandle);
	if (!infosize) return NULL;

	void* dataptr = malloc(infosize);
	if (!GetFileVersionInfo((LPTSTR)pszName, NULL, infosize, dataptr)) {
		free(dataptr);
		return NULL;
	}

	DWORD trans = 0;
	UINT cc;
	void* pv;
	if (VerQueryValue(dataptr, _T("\\VarFileInfo\\Translation"), &pv, &cc) && cc >= 4)
		trans = *((DWORD*)pv);

	void* temp = realloc(dataptr, sizeof(VER_INFO) + infosize);
	if (!temp) {
		free(dataptr);
		return NULL;
	}
	dataptr = temp;
	VS_VERSION* verptr = (VS_VERSION*)((BYTE*)dataptr + sizeof(VER_INFO));
	memmove(verptr, dataptr, infosize);

	VER_INFO* pvi = (VER_INFO*)dataptr;
	pvi->ressize = infosize;
	pvi->pVersionResource = verptr;
	pvi->dwTranslation = trans;
	pvi->pSignature = FindSignature(verptr, infosize);

	return (HANDLE)dataptr;
}

extern "C"
void WINAPI lccVerClose(HANDLE hMemVer)
{
	free((void*)hMemVer);
}

extern "C"
BOOL WINAPI lccVerGetVersion(HANDLE hMemVer, int idVersion, LPTSTR pszVersion)
{
	*pszVersion = 0;
	VER_INFO* pvi = (VER_INFO*)hMemVer;
	VS_FIXEDFILEINFO* info = pvi->pSignature;
	if (info) {
		if (idVersion == KV_PRODUCTVERSION) {
			wsprintf(pszVersion, _T("%d.%d.%d.%d"),
					HIWORD(info->dwProductVersionMS),
					LOWORD(info->dwProductVersionMS),
					HIWORD(info->dwProductVersionLS),
					LOWORD(info->dwProductVersionLS));
		}
		else if (idVersion == KV_FILEVERSION) {
			wsprintf(pszVersion, _T("%d.%d.%d.%d"),
					HIWORD(info->dwFileVersionMS),
					LOWORD(info->dwFileVersionMS),
					HIWORD(info->dwFileVersionLS),
					LOWORD(info->dwFileVersionLS));
		}
	}
	return (*pszVersion != 0);
}

extern "C"
BOOL WINAPI lccVerGetFileOS(HANDLE hMemVer, LPTSTR pszOS)
{
	*pszOS = 0;
	DWORD flags = lccVerGetFileOSFlags(hMemVer);
	if (flags != 0xFFFFFFFF) {
		wsprintf(pszOS, _T("%s / %s"),
					cps_lookup(cpOSType, (int)HIWORD(flags)),
					cps_lookup(cpOSBase, (int)LOWORD(flags)));
	}
    return (*pszOS != 0);
}

extern "C"
DWORD WINAPI lccVerGetFileOSFlags(HANDLE hMemVer)
{
	VER_INFO* pvi = (VER_INFO*)hMemVer;
	VS_FIXEDFILEINFO* info = pvi->pSignature;
	DWORD flags = 0xFFFFFFFF;
	if (info)
		flags = info->dwFileOS;
	return flags;
}

extern "C"
BOOL WINAPI lccVerGetFileType(HANDLE hMemVer, LPTSTR pszType)
{
	*pszType = 0;
	VER_INFO* pvi = (VER_INFO*)hMemVer;
	VS_FIXEDFILEINFO* info = pvi->pSignature;
	if (info)
		lstrcpy(pszType, cps_lookup(cpFileType, info->dwFileType));
    return (*pszType != 0);
}

extern "C"
BOOL WINAPI lccVerQueryValueEx(HANDLE hMemVer, LPCTSTR pszValueName, LPTSTR pszText, int cchMax)
{
	CString str;
	if (IS_INTRESOURCE(pszValueName)) {
		int idValue = (int)(INT_PTR)pszValueName;
		if (idValue < KV_COMMENTS || idValue > KV_SPECIALBUILD)
			return FALSE;
		str = szValue[idValue];
	}
	else
		str = pszValueName;

	VER_INFO* pvi = (VER_INFO*)hMemVer;
	TCHAR buf[512];
	wsprintf(buf, _T("\\StringFileInfo\\%04X%04X\\%s"), LOWORD(pvi->dwTranslation), HIWORD(pvi->dwTranslation), (LPCTSTR)str);

	void* pv = NULL;
	UINT cc = 0;
	if (VerQueryValue(pvi->pVersionResource, buf, &pv, &cc)) {
		lstrcpyn(pszText, (LPTSTR)pv, cchMax);
		return TRUE;
	}
	return FALSE;
}

extern "C"
BOOL WINAPI lccVerQueryValue(HANDLE hMemVer, int idValue, LPTSTR pszText)
{
	return lccVerQueryValueEx(hMemVer, MAKEINTRESOURCE(idValue), pszText, MAX_PATH);
}

extern "C"
BOOL WINAPI lccVerGetVersionEx(LPCTSTR pszName, LPDWORD pdwMajor, LPDWORD pdwMinor)
{
	*pdwMajor = 0;
	*pdwMinor = 0;
	HANDLE hmem = lccVerOpen(pszName);
	if (hmem) {
		VER_INFO* pvi = (VER_INFO*)hmem;
		VS_FIXEDFILEINFO* info = pvi->pSignature;
		if (info) {
			*pdwMajor = info->dwProductVersionMS;
			*pdwMinor = info->dwProductVersionLS;
		}
		lccVerClose(hmem);
		return TRUE;
	}
	return FALSE;
}
