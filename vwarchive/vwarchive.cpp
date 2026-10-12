// vwarc.cpp : DLL の初期化ルーチンです。
//

#include "stdafx.h"
#include "resource.h"
#include "common.h"

COLORREF gColors[5];

static void InitApplication(HINSTANCE hInst);
static void ExitApplication();


BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call,  LPVOID lpReserved)
{
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		InitApplication((HINSTANCE)hModule);
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		ExitApplication();
		break;
	}
	return TRUE;
}

static void InitApplication(HINSTANCE hInst)
{
	::DefWindowProc(NULL, 0, 0, 0L);
	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);
	_Module.Init(NULL, hInst);

	CAppReg reg;
	if (reg.GetBinary(IDS_SEC_OPERATION, IDS_KEY_COLORS, gColors, sizeof(gColors)) != sizeof(gColors)) {
		gColors[colText] = GetSysColor(COLOR_WINDOWTEXT);
		gColors[colBack] = GetSysColor(COLOR_WINDOW);
		gColors[colRText] = GetSysColor(COLOR_HIGHLIGHTTEXT);
		gColors[colRBack] = GetSysColor(COLOR_HIGHLIGHT);
		gColors[colInact] = GetSysColor(COLOR_3DFACE);
	}
}

static void ExitApplication()
{
	_Module.Term();
}
