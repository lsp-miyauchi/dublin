// pixeflt.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "..\..\fv7\oeminc.h"
#include "pixeflt.h"

#pragma comment(lib, "shlwapi.lib")


CAppModule _Module;


static BOOL GetLibraryPath(LPTSTR pszPath)
{
	*pszPath = '\0';

	TCHAR buf[MAX_PATH * 2];
	if (SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES_COMMON, NULL, SHGFP_TYPE_CURRENT, buf) == S_OK) {
		PathAppend(buf, APP_COMPANY);
		if (PathIsDirectory(buf)) {
			lstrcpy(pszPath, buf);
			return TRUE;
		}
	}
	return FALSE;
}

static const TCHAR envPath[] = _T("Path");

static void AddPathEnvironment(LPCTSTR pszAdd)
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

int Run(LPTSTR lpCmdLine, int nCmdShow)
{
	TCHAR cmd[MAX_PATH * 3];
	if (!GetLibraryPath(cmd)) return -1;

	AddPathEnvironment(cmd);

	TCHAR dll[MAX_PATH];
	lstrcpy(dll, cmd);
	PathAppend(dll, DLL_GMIL);
	if (!PathFileExists(dll)) return -1;

	HMODULE hmod = LoadLibrary(dll);
	if (!hmod) return -1;

	bool bViewer = false;
	void (WINAPI* __run)(HWND, HINSTANCE, LPCTSTR, int);
	TCHAR dir[MAX_PATH * 2];
	GetCurrentDirectory(ARRAYSIZE(dir), dir);
	if (lpCmdLine && *lpCmdLine) {
		// コマンドオプションをスキップしてターゲットファイルのフォルダを取得
		TCHAR* p = lpCmdLine;
		while (p && (*p == '/' || *p == '-')) {
			p++;
			if (*p == 'r' || *p == 'R' || *p == 'v' || *p == 'V')
				bViewer = true;
			if (*p == 'e' || *p == 'E')
				bViewer = false;
			p = StrChr(p, ' ');
			while (p && *p == ' ') p++;
		}
		if (p && *p) {
			lstrcpy(dir, p);
			PathUnquoteSpaces(dir);
			if (PathFileExists(dir)) {
				PathRemoveFileSpec(dir);
				SetCurrentDirectory(dir);
			}
		}
	}

	(FARPROC&)__run = GetProcAddress(hmod, bViewer ? "imgRunW" : "pxfRunW");
	if (__run)
		__run(NULL, MODINST, lpCmdLine, SW_SHOWNORMAL);
	FreeLibrary(hmod);
	return 0;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HRESULT hRes = ::CoInitialize(NULL);
	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);

	hRes = _Module.Init(NULL, hInstance);
	int nRet = Run(lpstrCmdLine, nCmdShow);
	_Module.Term();

	::CoUninitialize();
	return nRet;
}
