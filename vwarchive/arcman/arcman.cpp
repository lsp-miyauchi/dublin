// arcman.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "..\..\fv7\oeminc.h"
#include "arcman.h"

#pragma comment(lib, "shlwapi.lib")

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
static const TCHAR delim[] = _T(";");

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
	int len = lstrlen(path);
	GetShortPathName(path, spath, ARRAYSIZE(spath));

	DWORD size = GetEnvironmentVariable(envPath, NULL, 0);
	if (size > 0) {
		TCHAR* env = new TCHAR[size + len + 16];
		GetEnvironmentVariable(envPath, env, size + 16);

		bool found = false;
		TCHAR* ctx = NULL;
		TCHAR* token = _tcstok_s(env, delim, &ctx);
		while (token != NULL) {
			TCHAR t[MAX_PATH];
			lstrcpy(t, token);
			PathRemoveBackslash(t);
			if (!StrCmpI(t, path) || !StrCmpI(t, spath)) {
				found = true;
				break;
			}
			token = _tcstok_s(NULL, delim, &ctx);
		}

		if (!found) {
			lstrcpy(env, path);
			lstrcat(env, delim);
			len = lstrlen(env);

			// 再取得
			GetEnvironmentVariable(envPath, env + len, size + 1);
			SetEnvironmentVariable(envPath, env);
		}
		delete [] env;
	}
}

int APIENTRY _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int nCmdShow)
{
	TCHAR cmd[MAX_PATH * 3], dll[MAX_PATH];

	// 引数がなければ終了
	if (!lpCmdLine || *lpCmdLine == 0) {
		LoadString(hInst, IDS_APP_TITLE, dll, ARRAYSIZE(dll));
		LoadString(hInst, IDS_NOARGUMENTS, cmd, ARRAYSIZE(cmd));
		MessageBox(NULL, cmd, dll, MB_OK | MB_ICONSTOP);
		return -1;
	}

	if (!GetLibraryPath(cmd)) return -1;

	AddPathEnvironment(cmd);

	lstrcpy(dll, cmd);
	PathAppend(dll, DLL_ARC);
	if (!PathFileExists(dll)) return -1;

	HMODULE hmod = LoadLibrary(dll);
	if (!hmod) return -1;

	void (WINAPI* __run)(HWND, HINSTANCE, LPCTSTR, int);
	(FARPROC&)__run = GetProcAddress(hmod, "RunViewW");
	if (__run) {
		TCHAR dir[MAX_PATH * 2];
		GetCurrentDirectory(ARRAYSIZE(dir), dir);

		lstrcpy(cmd, lpCmdLine);
		LPTSTR p = cmd;
		while (*p == ' ') p++;
		if (*p == '"') PathUnquoteSpaces(p);

		if (*p && PathFileExists(p)) {
			lstrcpy(dir, p);
			PathRemoveFileSpec(dir);
			SetCurrentDirectory(dir);
			__run(NULL, hInst, p, SW_SHOWNORMAL);
		}
	}
	FreeLibrary(hmod);
	return 0;
}
