// execdll.cpp : main source file for execdll.exe
//

#include "stdafx.h"
#include "resource.h"

CAppModule _Module;

const TCHAR szTitle[] = _T("execdll");						// タイトル バーのテキスト
const TCHAR szWindowClass[] = _T("lsp_execdll");			// メイン ウィンドウ クラス名

static BOOL GetLibraryPath(CString& str)
{
	str.Empty();
	TCHAR buf[MAX_PATH * 2];
	if (SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES_COMMON, NULL, SHGFP_TYPE_CURRENT, buf) == S_OK) {
		PathAppend(buf, _T("LightShip"));
		if (PathIsDirectory(buf)) {
			str = buf;
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

ATOM MyRegisterClass(HINSTANCE hInstance, LPCTSTR pszIcon)
{
	UINT idIcon = IDI_EDIT;
	if (pszIcon && *pszIcon) {
		if      (!lstrcmpi(pszIcon, _T("search")))		idIcon = IDI_SEARCH;
		else if (!lstrcmpi(pszIcon, _T("image")))		idIcon = IDI_IMAGE;
		else if (!lstrcmpi(pszIcon, _T("archive")))		idIcon = IDI_ARCHIVE;
		else if (!lstrcmpi(pszIcon, _T("sync")))		idIcon = IDI_SYNC;
		else if (!lstrcmpi(pszIcon, _T("bdump")))		idIcon = IDI_BDUMP;
		else if (!lstrcmpi(pszIcon, _T("edit")))		idIcon = IDI_EDIT;
	}
	WNDCLASSEX wcex;
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= DefWindowProc;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(idIcon));
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

static int AppMessage(UINT idResource, LPCTSTR pszInsert, UINT style)
{
	CString tit; tit.LoadString(IDR_MAINFRAME);
	CString str; str.Format(idResource, pszInsert);
	return ::MessageBox(::GetActiveWindow(), str, tit, style);
}

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE /*hPrevInstance*/, LPTSTR lpCmdLine, int nCmdShow)
{
	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls
	hRes = _Module.Init(NULL, hInst);
	ATLASSERT(SUCCEEDED(hRes));

	if (lpCmdLine && *lpCmdLine) {
		CString str = lpCmdLine;
		CString icon;
		int pos = 0;
		CString dll = str.Tokenize(_T(","), pos);
		if (dll.GetAt(0) == _T('/')) {
			icon = dll.Mid(1);
			dll = str.Tokenize(_T(","), pos);
		}
		if (dll != _T("")) {
			dll.Trim(_T(" \""));
			str = str.Mid(pos);
			pos = 0;
			CString arg;
			CString fn = str.Tokenize(_T(" "), pos);
			if (fn != _T(""))
				arg = str.Mid(pos);
			else
				fn = str;

			if (GetLibraryPath(str))
				AddPathEnvironment(str);

			// グローバル文字列を初期化しています。
			MyRegisterClass(hInst, icon);

			HWND hwnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, NULL, NULL);
			if (hwnd) {
				HMODULE hmod = LoadLibrary(dll);
				if (hmod) {
					CStringA fnc = fn; fnc += "W";
					bool bFoundFunction = false;
					void (WINAPI* pfn)(HWND, HINSTANCE, void*, int);
					(FARPROC&)pfn = GetProcAddress(hmod, fnc);
					if (pfn) {
						bFoundFunction = true;
						(*pfn)(hwnd, hInst, (void*)arg.GetString(), SW_SHOWNORMAL);
					}
					else {
						fnc = fn;
						(FARPROC&)pfn = GetProcAddress(hmod, fnc);
						if (pfn) {
							bFoundFunction = true;
							CStringA argA = arg;
							(*pfn)(hwnd, hInst, (void*)argA.GetString(), SW_SHOWNORMAL);
						}
					}
					FreeLibrary(hmod);

					if (!bFoundFunction)
						AppMessage(IDS_NOFUNCTION, fn, MB_OK | MB_ICONSTOP);
				}
				else
					AppMessage(IDS_CANNOTLOAD, dll, MB_OK | MB_ICONSTOP);
				DestroyWindow(hwnd);
			}
			else
				AppMessage(IDS_INITFAILED, NULL, MB_OK | MB_ICONSTOP);
		}
	}

	_Module.Term();
	::CoUninitialize();

	return 0;
}
