// Fvlch7.cpp : main source file for Fvlch7.exe
//

#include "stdafx.h"
#include "resource.h"
#include "MainFrm.h"

////////////////////////////////////////////////////////////////////////
//

CAppModule _Module;
CConsole dbg;
COSPlatform gosp;
UINT nRM_SHNOTIFY = RegisterWindowMessage(_T("wm_shellnotify"));

#define WS_FVLSHWINDOW		(WS_THICKFRAME)


////////////////////////////////////////////////////////////////////////
//

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainFrame wndMain;
	wndMain.m_nCmdShow = nCmdShow;

	if (wndMain.CreateEx(NULL, NULL, WS_POPUP | WS_THICKFRAME, WS_EX_TOOLWINDOW | WS_EX_LAYERED) == NULL) {
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}
	wndMain.ShowWindow(wndMain.m_nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();

	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	// 既に存在すればそちらをアクティブに
	HWND hwnd = ::FindWindow(FRAMECLASS, NULL);
	if (hwnd) {
		SetForegroundWindow(hwnd);
		return 0;
	}

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
