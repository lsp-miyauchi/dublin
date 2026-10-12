// runmanager.cpp : main source file for wtltest.exe
//

#include "stdafx.h"

#include "resource.h"
#include "common.h"
#include "mainfrm.h"
#include "..\lscore\sharedmem.h"

CAppModule _Module;

////////////////////////////////////////////////////////////////////////
//

class ole_using
{
public:
	ole_using() { ::CoInitializeEx(NULL, COINIT_MULTITHREADED); }
	~ole_using() { ::CoUninitialize(); }
};


////////////////////////////////////////////////////////////////////////
//

static int Run(LPTSTR lpstrCmdLine, int nCmdShow)
{
//@	ole_using ou;
	gdiplus_using gu;

	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainFrame wndFrame;
	if (wndFrame.CreateEx() == NULL) {
		ATLTRACE(_T("Frame window creation failed!\n"));
		return 0;
	}

	wndFrame.ShowWindow(nCmdShow);
	wndFrame.UpdateWindow();

	// ��������
	if (lpstrCmdLine && *lpstrCmdLine) {
		TCHAR name[MAX_PATH * 2];
		lstrcpy(name, lpstrCmdLine);
		PathUnquoteSpaces(name);
		GetLongPathName(name, name, ARRAYSIZE(name));
		wndFrame.FileOpen(name);
	}

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

/////////////////////////////////////////////////////////////////////
//

extern "C"
void CALLBACK RunViewW(HWND hwnd, HINSTANCE hInst, LPWSTR lpCmdLine, int nCmdShow)
{
	Run(lpCmdLine, nCmdShow);
}

extern "C"
void CALLBACK RunView(HWND hwnd, HINSTANCE hInst, LPSTR lpCmdLine, int nCmdShow)
{
	WCHAR wz[MAX_PATH * 4];
	MultiByteToWideChar(CP_ACP, 0, lpCmdLine, -1, wz, ARRAYSIZE(wz));
	RunViewW(hwnd, hInst, wz, nCmdShow);
}

