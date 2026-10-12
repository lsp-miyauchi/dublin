// vwbin.cpp : DLL アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "common.h"
#include "cdll.h"
#include "resource.h"

BOOL gbValidUser = FALSE;
BOOL gbReadOnly = FALSE;
BOOL gbInsert = TRUE;

COSPlatform gosp;

#ifdef _USRDLL

static void InitApplication(HINSTANCE hInst);
static void ExitApplication();

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call,  LPVOID lpReserved)
{
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:	InitApplication((HINSTANCE)hModule); break;
	case DLL_THREAD_ATTACH:		break;
	case DLL_THREAD_DETACH:		break;
	case DLL_PROCESS_DETACH:	ExitApplication(); break;
	}
	return TRUE;
}

static void InitApplication(HINSTANCE hInst)
{
	::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	::DefWindowProc(NULL, 0, 0, 0L);
	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);
	_Module.Init(NULL, hInst);
}

static void ExitApplication()
{
	_Module.Term();
	::CoUninitialize();
}

#else

int Run(LPTSTR pstrCmdLine, int nCmdShow);

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	::CoUninitialize();

	return nRet;
}

#endif		// _USRDLL

int AppMessage(UINT nID, LPCTSTR pszInsert /*=NULL*/, UINT nType /*=MB_STOP*/)
{
	CString tit;
	tit.LoadString(IDS_APP_TITLE);
	CString str;
	str.LoadString(nID);
	if (pszInsert && lstrlen(pszInsert))
		str.Replace(_T("%s"), pszInsert);
	return ::MessageBox(::GetActiveWindow(), str, tit, nType);
}

int LastError(LPCTSTR pszTitle, UINT nType /*=MB_STOP*/)
{
	int nError = GetLastError();
	CString tit;
	tit.LoadString(IDS_APP_TITLE);

	return lccErrorBox(::GetActiveWindow(), nError, tit, pszTitle, nType);
}

void PathDecorate(LPTSTR psz)
{
	if (!psz || *psz == 0) return;

	LPTSTR name = StrDup(psz);
	CString ext;
	LPTSTR p = PathFindExtension(name);
	if (*p == '.') {
		ext = p;
		*p = 0;
	}

	CString body;
	for (int i = 1; i < 1000; i++) {
		body.Format(_T("%s[%d]"), name, i);
		if (!ext.IsEmpty())
			body += ext;
		if (!PathFileExists(body)) {
			lstrcpy(psz, body);
			break;
		}
	}
	LocalFree(name);
}

IShellItem* CreateShellItemFromPath(LPCTSTR pszFileName)
{
	if (!pszFileName || *pszFileName == 0) return NULL;

	IShellItem* psi = NULL;
	LPITEMIDLIST pidlFullyQual = ILCreateFromPath(pszFileName);
	if (pidlFullyQual) {
		IShellFolder* psf;
		LPCITEMIDLIST pidlSelf;
		if (SUCCEEDED(SHBindToParent(pidlFullyQual, IID_IShellFolder, (void**)&psf, &pidlSelf))) {
			if (FAILED(SHCreateShellItem(NULL, psf, pidlSelf, &psi)))
				psi = NULL;
			psf->Release();
		}
		CoTaskMemFree(pidlFullyQual);
	}
	return psi;
}

bool ShowOpenSaveFileDialog(HWND hwnd, bool bOpen, LPCTSTR pszTitle, LPCTSTR pszDir, LPTSTR pszFile, int cchMax)
{
	if (!pszFile) return false;

	COMDLG_FILTERSPEC rgSpec[] = { { L"すべて(*.*)", L"*.*" }, };

	CString strTitle;
	if (pszTitle && IS_INTRESOURCE(pszTitle)) {
		strTitle.LoadString(PtrToUint(pszTitle));
		pszTitle = strTitle;
	}

	// 初期フォルダ
	TCHAR dir[MAX_PATH * 2];
	if (pszDir && *pszDir)
		lstrcpy(dir, pszDir);
	else if (*pszFile) {
		lstrcpy(dir, pszFile);
		PathRemoveFileSpec(dir);
	}
	else
		dir[0] = 0;

	bool ok = false;
	IFileDialog* pfd;
	HRESULT hr = CoCreateInstance(
						bOpen ? CLSID_FileOpenDialog : CLSID_FileSaveDialog,
						NULL, CLSCTX_INPROC_SERVER, IID_IFileDialog, (void**)&pfd);
	if (SUCCEEDED(hr)) {
		if (*pszFile)
			pfd->SetFileName(pszFile);
		if (pszTitle)
			pfd->SetTitle(pszTitle);

		pfd->SetFileTypes(1, rgSpec);
		pfd->SetFileTypeIndex(1);					// one base, not zero base

		IShellItem* psi = CreateShellItemFromPath(dir);
		if (psi)
			pfd->SetDefaultFolder(psi);

		hr = pfd->Show(hwnd);
		if (SUCCEEDED(hr)) {
			IShellItem* psiResult;
			hr = pfd->GetResult(&psiResult);
			if (SUCCEEDED(hr)) {
				LPTSTR psz;
				hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &psz);
				if (SUCCEEDED(hr)) {
					lstrcpyn(pszFile, psz, cchMax);
					CoTaskMemFree(psz);
				}
				psiResult->Release();
			}
		}

		if (psi)
			psi->Release();
		pfd->Release();
		ok = SUCCEEDED(hr) ? true : false;
	}
	else {
		CFileDialog dlg(bOpen, 
						NULL,			// DefExt
						pszFile,
						OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
						_T("すべて(*.*)\0*.*\0\0"),
						hwnd);
		if (pszTitle)
			dlg.m_ofn.lpstrTitle = pszTitle;
		if (dir[0])
			dlg.m_ofn.lpstrInitialDir = dir;
		if (dlg.DoModal() == IDOK) {
			lstrcpyn(pszFile, dlg.m_szFileName, cchMax);
			ok = true;
		}
	}
	return ok;
}

void BootNewProcess(HWND hwnd, LPCTSTR cmd)
{
#ifdef _USRDLL
	lccDllExecute(hwnd, DLL_BED, BED_EDIT, cmd, _T("/bdump"), FALSE);
#else
	TCHAR name[MAX_PATH * 2];
	GetModuleFileName(NULL, name, ARRAYSIZE(name));
	TCHAR path[MAX_PATH * 2];
	lstrcpy(path, name);
	PathRemoveFileSpec(path);

	SHELLEXECUTEINFO sei;
	memset(&sei, 0, sizeof(sei));
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_UNICODE;
	sei.hwnd = hwnd;
	sei.nShow = SW_SHOWNORMAL;
	sei.lpVerb = NULL;
	sei.lpFile = name;
	sei.lpParameters = cmd;
	sei.lpDirectory = path;
	ShellExecuteEx(&sei);
#endif
}

