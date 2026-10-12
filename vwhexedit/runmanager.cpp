// runmanager.cpp : main source file for wtltest.exe
//

#include "stdafx.h"

#include "resource.h"

#include "hexmanager.h"
#include "beview.h"
#include "mainfrm.h"

CAppModule _Module;

int Run(LPTSTR lpstrCmdLine, int nCmdShow)
{
	gdiplus_using gu;

	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainFrame wndFrame;
	wndFrame.SetCommandLine(lpstrCmdLine);

	if(wndFrame.CreateEx() == NULL) {
		ATLTRACE(_T("Frame window creation failed!\n"));
		return 0;
	}

	wndFrame.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

////////////////////////////////////////////////////////////////////////

#ifdef _USRDLL

extern "C"
void WINAPI RunEditW(HWND hwnd, HINSTANCE hInst, LPCWSTR lpCmdLine, int nCmdShow)
{
	Run((LPWSTR)lpCmdLine, nCmdShow);
}

extern "C"
void WINAPI RunEdit(HWND hwnd, HINSTANCE hInst, LPCSTR lpCmdLine, int nCmdShow)
{
	WCHAR wsz[MAX_PATH * 2];
	MultiByteToWideChar(CP_ACP, 0, lpCmdLine, -1, wsz, MAX_PATH * 2);
	RunEditW(hwnd, hInst, wsz, nCmdShow);
}

#endif	// _USRDLL
