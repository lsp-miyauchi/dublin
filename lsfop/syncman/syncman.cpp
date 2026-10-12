// syncman.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "..\..\fv7\oeminc.h"
#include "syncman.h"

#pragma comment(lib, "shlwapi.lib")


CAppModule _Module;


class dynlib
{
private:
	HMODULE m_hmod;
public:
	dynlib(LPCTSTR pszDll = NULL) { m_hmod = (pszDll && *pszDll) ? LoadLibrary(pszDll) : NULL; }
	~dynlib() { if (m_hmod) FreeLibrary(m_hmod); }
	inline bool isloaded() { return (m_hmod != NULL); }
	inline FARPROC findproc(const char* pszFunc) { return (m_hmod ? GetProcAddress(m_hmod, pszFunc) : NULL); }
};

class cmdarray : public CSimpleArray<CString>
{
public:
	int parse(LPCTSTR pszCmdLine)
	{
		RemoveAll();
		if (!pszCmdLine || *pszCmdLine == 0) return 0;

		int len = lstrlen(pszCmdLine);
		TCHAR* base = new TCHAR[(len + 16) * 2];
		LPTSTR p = base;
		lstrcpy(p, pszCmdLine);
		LPTSTR arg = base + len + 16;
		while (*p) {
			while (*p == ' ') p++;

			LPTSTR q = arg;
			while (*p && *p != ' ') {
				if (*p == '"') {
#ifdef REMOVE_QUOTE
					p++;
					while (*p && *p != '"')
						*q++ = *p++;
					if (*p == '"')
						p++;
#else
					*q++ = *p++;
					while (*p && *p != '"')
						*q++ = *p++;
					if (*p == '"')
						*q++ = *p++;
#endif
				}
				else
					*q++ = *p++;
			}
			*q = 0;

			if (*arg)
				Add(arg);
		}
		delete [] base;

		return GetSize();
	}

	BOOL IsOptionCommand(int index)
	{
		int cnt = GetSize();
		if (cnt > 0 && index >= 0 && index < cnt) {
			LPCTSTR p = (*this)[index];
			return (*p == '/' || *p == '-');
		}
		return FALSE;
	}
};

static BOOL GetLibraryPath(LPTSTR pszPath)
{
	*pszPath = 0;

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

BOOL GetFileInfo(LPCTSTR pszName, WIN32_FIND_DATA& wfd)
{
	HANDLE hFind = FindFirstFile(pszName, &wfd);
	if (hFind != INVALID_HANDLE_VALUE) {
		FindClose(hFind);
		return TRUE;
	}
	return FALSE;
}

static const char* sszRun[] = {
	"FolderSyncManagerW",
	"FolderSyncRunW",
	"FolderSyncScriptRunW",
	"FolderSyncShowReportW",
};

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

int Run(LPTSTR pszCmdline, int nCmdShow)
{
	TCHAR cmd[MAX_PATH * 3];
	if (!GetLibraryPath(cmd)) return -1;

	AddPathEnvironment(cmd);

	// lsfop.dllをロード
	PathAppend(cmd, DLL_FOPSRV);
	if (!PathFileExists(cmd)) return -1;

	dynlib lib(cmd);
	if (!lib.isloaded()) return -1;

	// コマンドライン解析
	if (pszCmdline && *pszCmdline) {
		lstrcpy(cmd, pszCmdline);
		TCHAR* p = StrChr(cmd, _T('%'));
		if (p) {
			int (WINAPI* __expand)(LPTSTR, int);
			(FARPROC&)__expand = lib.findproc("fopExpandSyncPath");
			if (__expand) (*__expand)(p, MAX_PATH);
			else return -1;
		}
	}
	else
		cmd[0] = 0;

	cmdarray cl;
	int cnt = cl.parse(cmd);
	TCHAR targ[MAX_PATH * 2];
	targ[0] = 0;
	for (int i = 0; i < cnt; i++) {
		if (!cl.IsOptionCommand(i)) {
			lstrcpy(targ, cl[i]);
			PathUnquoteSpaces(targ);
			break;
		}
	}

	const char* pszFunc = sszRun[0];
	WIN32_FIND_DATA wfd;
	if (targ[0] && GetFileInfo(targ, wfd)) {
		if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			LPCTSTR p = PathFindExtension(targ);
			if (!StrCmpI(p, _T(".fsd")))
				pszFunc = sszRun[1];
			else if (!StrCmpI(p, _T(".fss")))
				pszFunc = sszRun[2];
			else if (!StrCmpI(p, _T(".synclog")))
				pszFunc = sszRun[3];
			else
				return -1;				// 対象のファイルタイプが不明
		}
	}

	void (WINAPI* __run)(HWND, HINSTANCE, LPCTSTR, int);
	(FARPROC&)__run = lib.findproc(pszFunc);
	if (__run)
		__run(NULL, MODINST, cmd, SW_SHOWNORMAL);
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
