// runmanager.cpp : main source file for wtltest.exe
//

#include "stdafx.h"

#include "resource.h"
#include "common.h"
#include "mainfrm.h"
#include "viewfrm.h"
#include "slideshow.h"

CAppModule _Module;

enum {
	VW_EDITOR,
	VW_VIEWER,
	VW_SLIDESHOW,
};

static int Run(LPTSTR lpstrCmdLine, int nCmdShow, int typeView)
{
	gdiplus_using gu;

	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CViewFrame viewFrame;
	CMainFrame editFrame;
	CSlideFrame slideFrame;

	if (typeView == VW_VIEWER) {
		if (viewFrame.CreateEx() == NULL) {
			return 0;
		}
		viewFrame.ShowWindow(nCmdShow);
		viewFrame.UpdateWindow();
		viewFrame.OpenFile(lpstrCmdLine, true);
	}
	else if (typeView == VW_EDITOR) {
		if (editFrame.CreateEx() == NULL) {
			return 0;
		}
		editFrame.ShowWindow(nCmdShow);
		editFrame.UpdateWindow();
		editFrame.OpenFile(lpstrCmdLine);
	}

	int nRet = theLoop.Run();
	_Module.RemoveMessageLoop();
	return nRet;
}

////////////////////////////////////////////////////////////////
//

extern "C"
void WINAPI pxfRunW(HWND hwnd, HINSTANCE hInst, LPWSTR pCmd, int nCmdShow)
{
	Run((LPWSTR)pCmd, nCmdShow, VW_EDITOR);
}

extern "C"
void WINAPI pxfRun(HWND hwnd, HINSTANCE hInst, LPSTR pCmd, int nCmdShow)
{
	WCHAR wsz[MAX_PATH * 2];
	MultiByteToWideChar(CP_ACP, 0, pCmd, -1, wsz, _COUNTOF(wsz));
	pxfRunW(hwnd, hInst, wsz, nCmdShow);
}

extern "C"
void WINAPI slideRunW(HWND hwnd, HINSTANCE hInst, LPWSTR pCmd, int nCmdShow)
{
	gdiplus_using gu;

	TCHAR cmd[MAX_PATH * 2];
	lstrcpy(cmd, pCmd);
	PathUnquoteSpaces(cmd);

	CSlideFrame slideFrame;
	if (slideFrame.Initialize(cmd)) {
		if (slideFrame.Create() == NULL) {
			return;
		}
		slideFrame.MessageLoop();
	}
}

extern "C"
void WINAPI slideRun(HWND hwnd, HINSTANCE hInst, LPSTR pCmd, int nCmdShow)
{
	WCHAR cmd[MAX_PATH * 4];
	MultiByteToWideChar(CP_ACP, 0, pCmd, -1, cmd, ARRAYSIZE(cmd));
	slideRunW(hwnd, hInst, cmd, nCmdShow);
}

extern "C"
void WINAPI imgRunW(HWND hwnd, HINSTANCE hInst, LPWSTR pCmd, int nCmdShow)
{
	TCHAR* p = pCmd;
	while (p && (*p == '/' || *p == '-')) {
		p = StrChr(p, ' ');
		while (p && *p == ' ') p++;
	}

	TCHAR cmd[MAX_PATH * 2];
	if (p && *p) {
		lstrcpy(cmd, p);
		PathUnquoteSpaces(cmd);
	}
	else
		cmd[0] = 0;
	Run(cmd, nCmdShow, VW_VIEWER);
}

extern "C"
void WINAPI imgRun(HWND hwnd, HINSTANCE hInst, LPSTR pCmd, int nCmdShow)
{
	WCHAR cmd[MAX_PATH * 4];
	MultiByteToWideChar(CP_ACP, 0, pCmd, -1, cmd, ARRAYSIZE(cmd));
	imgRunW(hwnd, hInst, cmd, nCmdShow);
}

