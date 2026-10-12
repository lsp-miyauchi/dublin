#include "stdafx.h"
#include "sys\stat.h"
#include "lscore.h"
#include "common.h"
#include "fnc.h"

//////////////////////////////////////////////////////////////////////
//

extern "C"
int WINAPI lccIsPathInclude(LPCTSTR pszSrc, LPCTSTR pszDst)
// pszDstがpszSrcの子孫フォルダかどうかをチェックする
{
	TCHAR s[MAX_PATH * 2];
	TCHAR t[MAX_PATH * 2];

	lstrcpy(s, pszSrc);
	PathAddBackslash(s);
	CharUpper(s);

	lstrcpy(t, pszDst);
	PathAddBackslash(t);
	CharUpper(t);

	return StrCmpN(s, t, lstrlen(s));
}

extern "C"
int WINAPI lccComparePath(LPCTSTR pszSrc, LPCTSTR pszDst)
{
	TCHAR s[MAX_PATH * 2], t[MAX_PATH * 2];
	lstrcpy(s, pszSrc);
	lstrcpy(t, pszDst);
	PathRemoveBackslash(s);
	PathRemoveBackslash(t);
	lccReplaceChar(s, '\\', '\t');
	lccReplaceChar(t, '\\', '\t');

	return StrCmpLogicalW(s, t);
}

extern "C"
BOOL WINAPI lccGetModuleFilePath(HINSTANCE hInst, LPTSTR pszPath)
{
	TCHAR buf[MAX_PATH * 2];
	if (GetModuleFileName(hInst, buf, ARRAYSIZE(buf)) && PathRemoveFileSpec(buf)) {
		lstrcpy(pszPath, buf);
		return TRUE;
	}
	return FALSE;
}

extern "C"
BOOL WINAPI lccMakeTree(LPCTSTR pszPath)
{
	if (PathIsRoot(pszPath) || lstrlen(pszPath) < 3)
		return TRUE;

	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, pszPath);
	LPTSTR p = buf;
	if (_ISNETPATH(p))
	{
		p = PathFindNextComponent(p);	// \\をスキップ
		p = PathFindNextComponent(p);	// コンピュータ名をスキップ
		p = PathFindNextComponent(p);	// 共有名をスキップ
	}
	while (p && *p)
	{
		LPTSTR q = StrChr(p, '\\');
		if (q)
			*q = '\0';

		if (lstrlen(buf) >= 3 && !PathIsRoot(buf))
		{
			if (!_pathisdirectory(buf) && !CreateDirectory(buf, NULL))
				return FALSE;
		}
		if (q)
			*q++ = '\\';
		p = q;
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// SHLWAPI implement


extern "C"
BOOL WINAPI lccPathGetFileInformation(LPCTSTR pszName, WIN32_FIND_DATA* pwfd)
{
	if (!_getfileinfo(pszName, pwfd)) {
		memset(pwfd, 0, sizeof(WIN32_FIND_DATA));
		return FALSE;
	}
	return TRUE;
}

extern "C"
DWORD WINAPI lccPathGetFileAttributes(LPCTSTR pszName)
{
	WIN32_FIND_DATA wfd;
	return (_getfileinfo(pszName, &wfd) ? wfd.dwFileAttributes : 0xffffffff);
}

extern "C"
BOOL WINAPI lccPathBuild(LPCTSTR pszSrc, LPCTSTR pszBase, LPTSTR pszDst)
{
	if (!pszSrc || *pszSrc == '\0' || !pszBase || *pszBase == '\0' || !pszDst)
		return FALSE;

	// 作成ファイル名の解析
	TCHAR buf[MAX_PATH * 2], w[MAX_PATH * 2];
	lstrcpy(buf, pszSrc);
	if (!PathIsUNC(buf)) {								// ネットワークではない
		if (buf[0] == '.') {							// 相対パス
			lstrcpy(w, pszBase);
			PathAppend(w, buf);
			PathCanonicalize(buf, w);
		}
		else if (buf[0] == '\\') {						// ルートから
			lstrcpy(w, pszBase);
			PathStripToRoot(w);
			PathAppend(w, &buf[1]);
			lstrcat(buf, w);
		}
		else if (lstrlen(buf) < 2 || buf[1] != ':') {	// アペンド
			lstrcpy(w, pszBase);
			PathAppend(w, buf);
			lstrcpy(buf, w);
		}
	}
	lstrcpy(pszDst, buf);
	return TRUE;
}

static const TCHAR envPath[] = _T("Path");

extern "C"
void WINAPI lccAddPathEnvironment(LPCTSTR pszAdd)
{
	BOOL (WINAPI* _SetDllDirectory)(LPCWSTR);
	(FARPROC&)_SetDllDirectory = ::GetProcAddress(::GetModuleHandle(_T("kernel32")), "SetDllDirectoryW");
	if (_SetDllDirectory) {
		_SetDllDirectory(pszAdd);
		return;
	}

	TCHAR path[MAX_PATH], spath[MAX_PATH];
	lstrcpy(path, pszAdd);
	PathRemoveBackslash(path);
	GetShortPathName(path, spath, ARRAYSIZE(spath));

	DWORD size = GetEnvironmentVariable(envPath, NULL, 0);
	if (size > 0) {
		CString env;
		LPTSTR p = env.GetBuffer(size + 16);
		GetEnvironmentVariable(envPath, p, size + 16);
		env.ReleaseBuffer();

		bool found = false;
		int pos = 0;
		CString token = env.Tokenize(_T(";"), pos);
		while (token != _T("")) {
			TCHAR t[MAX_PATH];
			lstrcpy(t, token);
			PathRemoveBackslash(t);
			if (!lstrcmpi(t, path) || !lstrcmpi(t, spath)) {
				found = true;
				break;
			}
			token = env.Tokenize(_T(";"), pos);
		}

		if (!found) {
			CString newEnv = path;
			newEnv += _T(';');
			newEnv += env;
			SetEnvironmentVariable(envPath, newEnv);
		}
	}
}
