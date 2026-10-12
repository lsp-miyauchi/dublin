////////////////////////////////////////////////////////////////////////
// dllexports.cpp
//

#include "stdafx.h"
#include "fopstd.h"

////////////////////////////////////////////////////////////////////////
//

BOOL ExecSyncScript(HWND hwnd, LPCTSTR pszScriptFile);
BOOL ExecSyncCommand(HWND hwnd, LPCTSTR pszCmdline);


static BOOL sync_dir(HWND hwnd, LPCTSTR pszSyncFile)
{
	TCHAR file[MAX_PATH * 2];
	if (!pszSyncFile || *pszSyncFile == 0)
		file[0] = 0;
	else {
		LPCTSTR p = SkipTabSpace((LPTSTR)pszSyncFile);
		lstrcpy(file, p);
		if (*p != '"' && *p != '/' && *p != '-')
			PathQuoteSpaces(file);
	}
	return ExecSyncCommand(hwnd, file);
}

extern "C"
void WINAPI FolderSyncRunW(HWND hwnd, HINSTANCE hInst, LPCWSTR lpCmdLine, int nCmdShow)
{
	sync_dir(hwnd, lpCmdLine);
}

extern "C"
void WINAPI FolderSyncRun(HWND hwnd, HINSTANCE hInst, LPCSTR lpCmdLine, int nCmdShow)
{
	CStringW cmd(lpCmdLine);
	FolderSyncRunW(hwnd, hInst, cmd, nCmdShow);
}

extern "C"
void WINAPI FolderSyncScriptRunW(HWND hwnd, HINSTANCE hInst, LPCWSTR lpCmdLine, int nCmdShow)
{
	ExecSyncScript(hwnd, lpCmdLine);
}

extern "C"
void WINAPI FolderSyncScriptRun(HWND hwnd, HINSTANCE hInst, LPCSTR lpCmdLine, int nCmdShow)
{
	CStringW cmd(lpCmdLine);
	FolderSyncScriptRunW(hwnd, hInst, cmd, nCmdShow);
}

extern "C"
void WINAPI FolderSyncManagerW(HWND hwnd, HINSTANCE hInst, LPCWSTR lpCmdLine, int nCmdShow)
{
	WCHAR windir[MAX_PATH];
	WCHAR olddir[MAX_PATH * 2];
	GetWindowsDirectory(windir, ARRAYSIZE(windir));
	GetCurrentDirectory(ARRAYSIZE(olddir), olddir);
	SetCurrentDirectory(windir);

	sync_dir(hwnd, lpCmdLine);

	SetCurrentDirectory(olddir);
}

extern "C"
void WINAPI FolderSyncManager(HWND hwnd, HINSTANCE hInst, LPCSTR lpCmdLine, int nCmdShow)
{
	CStringW cmd(lpCmdLine);
	FolderSyncManagerW(hwnd, hInst, cmd, nCmdShow);
}

extern "C"
void WINAPI FolderSyncShowReportW(HWND hwnd, HINSTANCE hInst, LPCWSTR lpCmdLine, int nCmdShow)
{
	CPath name = lpCmdLine;
	name.UnquoteSpaces();
	TiXmlDocument* pXmlDoc = LoadSyncReport(name);
	if (pXmlDoc) {
		OpenSyncReport(hwnd, pXmlDoc);
		delete pXmlDoc;
	}
}

extern "C"
void WINAPI FolderSyncShowReport(HWND hwnd, HINSTANCE hInst, LPCSTR lpCmdLine, int nCmdShow)
{
	CStringW cmd(lpCmdLine);
	FolderSyncShowReportW(hwnd, hInst, cmd, nCmdShow);
}
