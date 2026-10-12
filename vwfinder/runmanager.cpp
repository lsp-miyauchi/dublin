// runmanager.cpp : main source file for wtltest.exe
//

#include "stdafx.h"

#include "resource.h"
#include "common.h"
#include "mainfrm.h"

CAppModule _Module;

static int Run(LPTSTR lpstrCmdLine, int nCmdShow)
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

	// ここでは画面表示はまだしない
	wndFrame.ShowWindow(SW_MINIMIZE);
	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

/////////////////////////////////////////////////////////////////////
//

extern "C"
void WINAPI finderRunW(HWND hwnd, HINSTANCE hInst, LPWSTR lpCmdLine, int nCmdShow)
{
	Run((LPTSTR)lpCmdLine, nCmdShow);
}

extern "C"
void WINAPI finderRun(HWND hwnd, HINSTANCE hInst, LPSTR lpCmdLine, int nCmdShow)
{
	WCHAR wz[MAX_PATH * 2];
	MultiByteToWideChar(CP_ACP, 0, lpCmdLine, -1, wz, _COUNTOF(wz));
	finderRunW(hwnd, hInst, wz, nCmdShow);
}
