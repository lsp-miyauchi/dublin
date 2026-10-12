#include "stdafx.h"

DWORD __fastcall _getfileinfo(LPCTSTR pszName, WIN32_FIND_DATA* pwfd)
{
	HANDLE hf = FindFirstFile(pszName, pwfd);
	DWORD code = GetLastError();
	if (hf != INVALID_HANDLE_VALUE)
		FindClose(hf);
	if (hf != INVALID_HANDLE_VALUE || code == ERROR_MORE_DATA)
		code = NOERROR;
	SetLastError(code);
	return code;
}

BOOL __fastcall GetFileData(LPCTSTR pszName, WIN32_FIND_DATA* pwfd)
{
	if (_getfileinfo(pszName, pwfd) == NOERROR) {
		// ローカル時間に変換
		FILETIME ft;
		FileTimeToLocalFileTime(&pwfd->ftCreationTime, &ft);
		pwfd->ftCreationTime = ft;
		FileTimeToLocalFileTime(&pwfd->ftLastWriteTime, &ft);
		pwfd->ftLastWriteTime = ft;
		FileTimeToLocalFileTime(&pwfd->ftLastAccessTime, &ft);
		pwfd->ftLastAccessTime = ft;
		return TRUE;
	}
	return FALSE;
}

BOOL __fastcall GetFileInfo(LPCTSTR pszName, CFileInfo& fi)
{
	CWin32FindData wfd;
	if (_getfileinfo(pszName, &wfd) == NOERROR) {
		TCHAR path[MAX_PATH * 2];
		PathGetParent(pszName, path);
		fi.SetFile(wfd, path);
		return TRUE;
	}
	return FALSE;
}


static void __fastcall build_unicode_path(LPCTSTR pSrc, LPWSTR pDst, int cchMax)
{
	TCHAR name[MAX_PATH * 2];
	lstrcpy(name, _T("\\\\?\\"));
	LPCTSTR p = pSrc;
	if (PathIsUNC(pSrc)) {
		p = PathFindNextComponent(pSrc);
		lstrcat(name, _T("UNC\\"));
	}
	lstrcat(name, p);
	lstrcpy(pDst, name);
}

static const TCHAR* invalid_names[] = {
	_T("CON"), _T("PRN"), _T("AUX"), _T("CLOCK$"), _T("NUL"),
	_T("COM1"), _T("COM2"), _T("COM3"), _T("COM4"), _T("COM5"), _T("COM6"), _T("COM7"), _T("COM8"), _T("COM9"),
	_T("LPT1"), _T("LPT2"), _T("LPT3"), _T("LPT4"), _T("LPT5"), _T("LPT6"), _T("LPT7"), _T("LPT8"), _T("LPT9"),
	NULL
};

BOOL __fastcall validate_makefile(LPTSTR pszName)
{
	// 文字列終端が ' ' または '.' で終わるフォルダ名の修正
	CString str = pszName;
	str.TrimRight(_T(" ."));
	if (str.IsEmpty())
		return FALSE;

	LPTSTR p = PathFindExtension(str.GetString());
	if (*p == '.')
		*p = 0;
	PathStripPath((LPTSTR)str.GetString());

	for (int i = 0; invalid_names[i] != NULL; i++) {
		if (!str.CompareNoCase(invalid_names[i]))
			return FALSE;
	}
	return TRUE;
}

HANDLE __fastcall _createfile(LPCTSTR pName, DWORD fdwAccess, DWORD fdwShareMode, LPSECURITY_ATTRIBUTES psa, DWORD fdwCreate, DWORD fdwAttrsAndFlags, HANDLE hTemplateFile)
{
	WCHAR wsz[MAX_PATH * 2];
	build_unicode_path(pName, wsz, ARRAYSIZE(wsz));
	return CreateFile(wsz, fdwAccess, fdwShareMode, psa, fdwCreate, fdwAttrsAndFlags, hTemplateFile);
}

BOOL __fastcall _createdirectory(LPCTSTR pName, LPSECURITY_ATTRIBUTES psa)
{
	TCHAR name[MAX_PATH];
	lstrcpy(name, pName);
	PathRemoveBlanks(name);

	WCHAR wsz[MAX_PATH * 2];
	build_unicode_path(name, wsz, ARRAYSIZE(wsz));
	return CreateDirectory(wsz, psa);
}

BOOL __fastcall _removedirectory(LPCTSTR pszPath)
{
	DWORD err;
	if (!RemoveDirectory(pszPath)) {
		err = GetLastError();
		WIN32_FIND_DATA wfd;
		if (_getfileinfo(pszPath, &wfd) == NOERROR) {
			TCHAR name[MAX_PATH * 2], dir[MAX_PATH * 2];
			PathCombine(name, pszPath, _T(".."));
			PathAppend(name, wfd.cAlternateFileName);
			PathCanonicalize(dir, name);
			return RemoveDirectory(dir);
		}
		else {
			SetLastError(err);
			return FALSE;
		}
	}
	return TRUE;
}

BOOL __fastcall _movefile(LPCTSTR pszExist, LPCTSTR pszNew)
{
	DWORD flg = MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH;
	if (!PathIsSameRoot(pszExist, pszNew))
		flg |= MOVEFILE_COPY_ALLOWED;
	BOOL bOK = MoveFileWithProgress(pszExist, pszNew, NULL, NULL, flg);
	if (!bOK && (GetLastError() == ERROR_NOT_SAME_DEVICE)) {
		flg |= MOVEFILE_COPY_ALLOWED;
		bOK = MoveFileWithProgress(pszExist, pszNew, NULL, NULL, flg);
	}
	return bOK;
}

void __fastcall ShowError(int nErrorCode)
{
	LPTSTR lp;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, nErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lp, 0, NULL);
	char sz[MAX_PATH * 2];
	WideCharToMultiByte(CP_OEMCP, 0, lp, -1, sz, ARRAYSIZE(sz), NULL, NULL);
	printf("err=%d:%s\n", nErrorCode, sz);
	LocalFree(lp);
}