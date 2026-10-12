#include "stdafx.h"
#include "cab.h"

///////////////////////////////////////////////////////////////////////
//

#define _ISNETPATH(a)	(wcslen((a)) > 3 && *(a)=='\\' && *((a)+1)=='\\' && *((a)+2)!='?')
#define _ISPATHSEP(a)	((a)=='\\'||(a)=='/')
#define _ISDIGIT(a)		((a)>='0'&&(a)<='9')

///////////////////////////////////////////////////////////////////////
// COMMON FUNCTIONS
//

int WStrToStr(LPCWSTR psz, CStringA& str)
{
	str.Empty();
	if (psz && *psz) {
		int len = WideCharToMultiByte(CP_ACP, 0, psz, -1, NULL, 0, NULL, NULL) + 8;
		LPSTR p = str.GetBuffer(len);
		WideCharToMultiByte(CP_ACP, 0, psz, -1, p, len, NULL, NULL);
		str.ReleaseBuffer();
	}
	return str.GetLength();
}

int StrToWStr(LPCSTR psz, CStringW& str)
{
	str.Empty();
	if (psz && *psz) {
		int len = MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0) + 8;
		LPWSTR p = str.GetBuffer(len);
		MultiByteToWideChar(CP_ACP, 0, psz, -1, p, len);
		str.ReleaseBuffer();
	}
	return str.GetLength();
}

bool IsLosslessUnicodeName(LPCWSTR pwzName)
{
	// UNICODE → ASCII → UNICODE でファイル名が変化するかどうかをチェック
	// 変化しなければ true 変化すれば false
	if (pwzName && *pwzName) {
		CStringA str;
		WStrToStr(pwzName, str);
		CStringW wstr;
		StrToWStr(str, wstr);
		return (_wcsicmp(wstr, pwzName) == 0);
	}
	return false;
}

int AppMessage(
			UINT idText,
			LPCWSTR pszInsert /*=NULL*/,
			UINT nType /*=MB_STOP*/)
{
	MessageBeep(0);

	CStringW msg;
	if (pszInsert && *pszInsert) {
		CStringW str;
		str.LoadString(idText);
		msg.Format(str, pszInsert);
	}
	else
		msg.LoadString(idText);

	CStringW cap;
	cap.LoadString(IDR_CAB_APP);

	return ::MessageBox(GetFocus(), msg, cap, nType);
}

BOOL GetFileData(
			LPCWSTR pszName,
			WIN32_FIND_DATA* pwfd)
{
	HANDLE hFind = FindFirstFile(pszName, pwfd);
	if (hFind != INVALID_HANDLE_VALUE) {
		FindClose(hFind);

		FILETIME ft;

		// ローカル時間に変換
		FileTimeToLocalFileTime(&pwfd->ftCreationTime, &ft);
		pwfd->ftCreationTime = ft;
		FileTimeToLocalFileTime(&pwfd->ftLastWriteTime, &ft);
		pwfd->ftLastWriteTime = ft;

		return TRUE;
	}
	return FALSE;
}


BOOL MakeTree(LPCWSTR pszPath)
{
	if (PathIsRoot(pszPath) || wcslen(pszPath) < 3)
		return TRUE;

	if (PathIsRelative(pszPath))
		return FALSE;

	WCHAR buf[MAX_PATH * 2];
	wcscpy(buf, pszPath);
	WCHAR* p = buf;
	if (PathIsUNC(p)) {
		p += 2;
		p = wcschr(++p, '\\');			// コンピュータ名をスキップ
		if (p)
			p = wcschr(++p, '\\');		// 共有名をスキップ
	}
	while (p && *p) {
		WCHAR* q = wcschr(p, '\\');
		if (q)
			*q = 0;

		if (wcslen(buf) >= 3 && !PathIsRoot(buf)) {
			if (!PathIsDirectory(buf)) {
				if (!CreateDirectory(buf, NULL))
					return FALSE;

				rSHChangeNotifyMakeDir(buf);
			}
		}
		if (q)
			*q++ = '\\';
		p = q;
	}
	return TRUE;
}

BOOL SetFileDateTime(
			LPCWSTR pszName,
			FILETIME* pftCreate,
			FILETIME* pftUpdate)
{
	DWORD attr = GetFileAttributes(pszName);
	if (attr == 0xFFFFFFFF)
		return FALSE;

	if (attr & FILE_ATTRIBUTE_DIRECTORY)
		return TRUE;

	if (!SetFileAttributes(pszName, FILE_ATTRIBUTE_NORMAL))
		return FALSE;

	BOOL bOK = FALSE;
	HANDLE hf = CreateFile(pszName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	if (hf != INVALID_HANDLE_VALUE) {
		FILETIME ftc, ftu;
		FILETIME* pC = NULL;
		FILETIME* pU = NULL;
		if (pftCreate) {
			LocalFileTimeToFileTime(pftCreate, &ftc);
			pC = &ftc;
		}
		if (pftUpdate) {
			LocalFileTimeToFileTime(pftUpdate, &ftu);
			pU = &ftu;
		}
		bOK = SetFileTime(hf, pC, NULL, pU);
		CloseHandle(hf);
	}
	SetFileAttributes(pszName, attr);
	return bOK;
}

LPWSTR FormatDosDate(WORD wDate, LPWSTR pszFormat)
{
	swprintf(pszFormat, L"%02u/%02u/%02u", (DTE_YEAR(wDate) + 80) % 100, DTE_MON(wDate), DTE_DAY(wDate));
	return pszFormat;
}

LPWSTR FormatDosTime(WORD wTime, LPWSTR pszFormat)
{
	swprintf(pszFormat, L"%02u:%02u:%02u", DTE_HOUR(wTime), DTE_MIN(wTime), DTE_SEC(wTime));
	return pszFormat;
}

BOOL RemoveFileNoConfirm(LPCWSTR pszName)
{
	SetFileAttributes(pszName, FILE_ATTRIBUTE_NORMAL);
	return DeleteFile(pszName);
}

void AttrToString(LPWSTR pszAttr, DWORD attr)
{
	LPWSTR p = pszAttr;
	*p++ = (attr & _A_SUBDIR)      ? 'd' : '-';
	*p++ = (attr & _A_SYSTEM)      ? 's' : '-';
	*p++ = (attr & _A_HIDDEN)      ? 'h' : '-';
	*p++ = (attr & _A_RDONLY)      ? 'r' : '-';
	*p++ = (attr & _A_ARCH)        ? 'a' : '-';
	*p++ = (attr & _A_NAME_IS_UTF) ? 'u' : '-';
	*p = 0;
}

///////////////////////////////////////////////////////////////////
//

#pragma pack(1)
typedef struct tagVS_VERSION {
    WORD  len;
    WORD  vlen;
} VS_VERSION;
#pragma pack()

static VS_FIXEDFILEINFO* __fastcall findsig(VS_VERSION* pvs)
{
	register DWORD* pdw = (DWORD*)pvs;
	register int i;
	for (i = 0; i < (pvs->len >> 2); i++) {
		if (*pdw == 0xFEEF04BD)
			break;
		pdw++;
	}
	if (i == (pvs->len >> 2))
		return NULL;
	return (VS_FIXEDFILEINFO*)pdw;
}

DWORD GetModuleVersion()
{
	DWORD version = 0xFFFFFFFF;
	WCHAR name[MAX_PATH * 2];
	if (GetModuleFileName(MODINST, name, ARRAYSIZE(name)) > 0) {
		DWORD dw;
		DWORD size = GetFileVersionInfoSize(name, &dw);
		if (size) {
			void* pi = malloc(size);
			if (pi) {
				if (GetFileVersionInfo(name, NULL, size, pi)) {
					VS_FIXEDFILEINFO* pvf = (VS_FIXEDFILEINFO*)findsig((VS_VERSION*)pi);
					if (pvf)
						version = pvf->dwProductVersionMS;
				}
				free(pi);
			}
		}
	}
	return version;
}

BOOL NotifyOwnerMessage(int mode, EXTRACTINGINFOEX* peie)
{
	HWND hOwner = _G.GetOwnerWindow();
	if (IsWindow(hOwner)) {
		if (gpfnProc)
			return (*gpfnProc)(hOwner, nRM_ARCEXTRACT, mode, peie);
		else
			return (BOOL)(_SNDMSG(hOwner, nRM_ARCEXTRACT, 1, peie) == 0);
	}
	return TRUE;
}

void rSHChangeNotifyMakeDir(LPCWSTR pszPath)
{
	if (pszPath && *pszPath && _fSHChangeNotify) {
		_fSHChangeNotify(SHCNE_MKDIR, SHCNF_PATH, pszPath, 0);
	}
}

