#include "stdafx.h"
#include "common.h"

#pragma comment(lib, "version.lib")


///////////////////////////////////////////////////////////////////////
// COMMON FUNCTIONS
//

bool IsLosslessUnicodeName(LPCWSTR pwzName)
{
	// UNICODE → ASCII → UNICODE でファイル名が変化するかどうかをチェック
	// 変化しなければ true 変化すれば false
	if (pwzName && *pwzName) {
		CStringA str(pwzName);
		CStringW wstr(str);
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
	cap.LoadString(IDR_APP);

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

void rSHChangeNotifyMakeDir(LPCWSTR pszPath)
{
	if (pszPath && *pszPath && _fSHChangeNotify)
		_fSHChangeNotify(SHCNE_MKDIR, SHCNF_PATH, pszPath, 0);
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

BOOL SetFileDateTimes(LPCTSTR pszName, DWORD attr, LPFILETIME pftCreate, LPFILETIME pftWrite)
{
	DWORD oldattr = GetFileAttributes(pszName);
	if (oldattr == 0xFFFFFFFF)
		return FALSE;
	if (!SetFileAttributes(pszName, oldattr & ~FILE_ATTRIBUTE_READONLY))
		return FALSE;
	BOOL bOK = FALSE;
	DWORD flags = 0;
	if (attr & FILE_ATTRIBUTE_DIRECTORY)
		flags |= FILE_FLAG_BACKUP_SEMANTICS;

	HANDLE hf = CreateFile(pszName, GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL, OPEN_EXISTING, flags, NULL);
	if (hf != INVALID_HANDLE_VALUE) {
		FILETIME ftc, ftw;
		LPFILETIME pc = NULL, pw = NULL, pa = NULL;
		if (pftCreate) {
			LocalFileTimeToFileTime(pftCreate, &ftc);
			pc = &ftc;
		}
		if (pftWrite) {
			LocalFileTimeToFileTime(pftWrite, &ftw);
			pw = &ftw;
		}
		bOK = SetFileTime(hf, pc, pa, pw);
		CloseHandle(hf);
	}
	if (attr == (DWORD)-1) attr = oldattr;
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
	*p++ = (attr & _A_CRPT)        ? 'c' : '-';
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
			return (BOOL)(SendMessage(hOwner, nRM_ARCEXTRACT, 1, (LPARAM)peie) == 0);
	}
	return TRUE;
}

